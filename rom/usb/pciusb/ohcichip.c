/*
    Copyright © 2010, The AROS Development Team. All rights reserved
    $Id$
*/

#include <proto/oop.h>
#include <hidd/pci.h>

#include <devices/usb_hub.h>

#include "uhwcmd.h"

#undef HiddPCIDeviceAttrBase
#define HiddPCIDeviceAttrBase (hd->hd_HiddPCIDeviceAB)
#undef HiddAttrBase
#define HiddAttrBase (hd->hd_HiddAB)

static AROS_UFH3(void, OhciResetHandler,
                 AROS_UFHA(struct PCIController *, hc, A1),
                 AROS_UFHA(APTR, unused, A5),
                 AROS_UFHA(struct ExecBase *, SysBase, A6))
{
    AROS_USERFUNC_INIT

    // reset controller
    CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS, OCSF_HCRESET);

    AROS_USERFUNC_EXIT
}

void ohciFreeEDContext(struct PCIController *hc, struct OhciED *oed) {

    struct OhciTD *otd;
    struct OhciTD *nextotd;

    KPRINTF(5, ("Unlinking EDContext %08lx\n", oed));

    // unlink from schedule
    oed->oed_Succ->oed_Pred = oed->oed_Pred;
    oed->oed_Pred->oed_Succ = oed->oed_Succ;
    oed->oed_Pred->oed_NextED = oed->oed_Succ->oed_Self;
    CacheClearE(&oed->oed_Pred->oed_EPCaps, 16, CACRF_ClearD);
    SYNC;

    Disable();
    nextotd = oed->oed_FirstTD;
    while(nextotd)
    {
        KPRINTF(1, ("FreeTD %08lx\n", nextotd));
        otd = nextotd;
        nextotd = (struct OhciTD *) otd->otd_Succ;
        ohciFreeTD(hc, otd);
    }

    ohciFreeED(hc, oed);
    Enable();
}

void ohciUpdateIntTree(struct PCIController *hc) {

    struct OhciED *oed;
    struct OhciED *predoed;
    struct OhciED *lastusedoed;
    UWORD cnt;

    // optimize linkage between queue heads
    predoed = lastusedoed = hc->hc_OhciTermED;
    for(cnt = 0; cnt < 5; cnt++)
    {
        oed = hc->hc_OhciIntED[cnt];
        if(oed->oed_Succ != predoed)
        {
            lastusedoed = oed->oed_Succ;
        }
        oed->oed_NextED = lastusedoed->oed_Self;
        CacheClearE(&oed->oed_EPCaps, 16, CACRF_ClearD);
        predoed = oed;
    }
}

void ohciHandleFinishedTDs(struct PCIController *hc) {

    struct PCIUnit *unit = hc->hc_Unit;
    struct IOUsbHWReq *ioreq;
    struct IOUsbHWReq *nextioreq;
    struct OhciED *oed = NULL;
    struct OhciTD *otd;
    UWORD devadrep;
    ULONG len;
    ULONG ctrlstatus;
    ULONG epcaps;
    BOOL direction_in;
    BOOL updatetree = FALSE;
    ULONG donehead;
    BOOL retire;

    KPRINTF(1, ("Checking for work done...\n"));
    Disable();
    donehead = hc->hc_OhciDoneQueue;
    hc->hc_OhciDoneQueue = 0UL;
    Enable();
    if(!donehead)
    {
        KPRINTF(1, ("Nothing to do!\n"));
        return;
    }
    otd = (struct OhciTD *) ((IPTR)donehead - hc->hc_PCIVirtualAdjust - 16);
    KPRINTF(10, ("DoneHead=%08lx, OTD=%08lx, Frame=%ld\n", donehead, otd, READREG32_LE(hc->hc_RegBase, OHCI_FRAMECOUNT)));
    do
    {
        CacheClearE(&otd->otd_Ctrl, 16, CACRF_InvalidateD);
        oed = otd->otd_ED;
        if(!oed)
        {
            KPRINTF(200, ("Came across a rogue TD that already has been freed!\n"));
            if(!otd->otd_NextTD)
            {
                break;
            }
            otd = (struct OhciTD *) ((IPTR)(READMEM32_LE(&otd->otd_NextTD) & OHCI_PTRMASK) - hc->hc_PCIVirtualAdjust - 16);
            continue;
        }
        CacheClearE(&oed->oed_EPCaps, 16, CACRF_InvalidateD);
        ctrlstatus = READMEM32_LE(&otd->otd_Ctrl);
        KPRINTF(1, ("TD: %08x - %08x\n", READMEM32_LE(&otd->otd_BufferPtr),
        		READMEM32_LE(&otd->otd_BufferEnd)));
        if(otd->otd_BufferPtr)
        {
            // FIXME this will blow up if physical memory is ever going to be discontinuous
            len = READMEM32_LE(&otd->otd_BufferPtr) - (READMEM32_LE(&otd->otd_BufferEnd) + 1 - otd->otd_Length);
        } else {
            len = otd->otd_Length;
        }

	/*
	 * CHECKME: ioreq was previously assigned AFTER the following check.
	 * However the check may use ioreq value itself, and it caused 'ioreq may be used uninitialized'
	 * warning. I hope this fix is correct.
	 */
        ioreq = oed->oed_IOReq;

        if (len)
        {
            epcaps = READMEM32_LE(&oed->oed_EPCaps);
            direction_in = ((epcaps & OECM_DIRECTION) == OECF_DIRECTION_TD)
                        ? (ioreq->iouh_SetupData.bmRequestType & URTF_IN)
                        : (epcaps & OECF_DIRECTION_IN);
            CachePostDMA((APTR)(IPTR)READMEM32_LE(&otd->otd_BufferEnd) - len + 1, &len, direction_in ? 0 : DMA_ReadFromRAM);
        }

        KPRINTF(1, ("Examining TD %08lx for ED %08lx (IOReq=%08lx), Status %08lx, len=%ld\n", otd, oed, ioreq, ctrlstatus, len));
        if(!ioreq)
        {
            KPRINTF(200, ("Came across a rogue IOReq that already has been replied!\n"));
            if(!otd->otd_NextTD)
            {
                break;
            }
            otd = (struct OhciTD *) ((IPTR)(READMEM32_LE(&otd->otd_NextTD) & OHCI_PTRMASK) - hc->hc_PCIVirtualAdjust - 16);
            continue;
        }
        ioreq->iouh_Actual += len;
        retire = (ioreq->iouh_Actual == ioreq->iouh_Length);
        if((ctrlstatus & OTCM_DELAYINT) != OTCF_NOINT)
        {
            retire = TRUE;
        }
        switch((ctrlstatus & OTCM_COMPLETIONCODE)>>OTCS_COMPLETIONCODE)
        {
            case (OTCF_CC_NOERROR>>OTCS_COMPLETIONCODE):
                break;

            case (OTCF_CC_CRCERROR>>OTCS_COMPLETIONCODE):
                KPRINTF(200, ("CRC Error!\n"));
                ioreq->iouh_Req.io_Error = UHIOERR_CRCERROR;
                retire = TRUE;
                break;

            case (OTCF_CC_BABBLE>>OTCS_COMPLETIONCODE):
                KPRINTF(200, ("Babble/Bitstuffing Error!\n"));
                ioreq->iouh_Req.io_Error = UHIOERR_CRCERROR;
                retire = TRUE;
                break;

            case (OTCF_CC_WRONGTOGGLE>>OTCS_COMPLETIONCODE):
                KPRINTF(200, ("Data toggle mismatch length = %ld\n", len));
                break;

            case (OTCF_CC_STALL>>OTCS_COMPLETIONCODE):
                KPRINTF(200, ("STALLED!\n"));
                ioreq->iouh_Req.io_Error = UHIOERR_STALL;
                retire = TRUE;
                break;

            case (OTCF_CC_TIMEOUT>>OTCS_COMPLETIONCODE):
                KPRINTF(200, ("TIMEOUT!\n"));
                ioreq->iouh_Req.io_Error = UHIOERR_TIMEOUT;
                retire = TRUE;
                break;

            case (OTCF_CC_PIDCORRUPT>>OTCS_COMPLETIONCODE):
                KPRINTF(200, ("PID Error!\n"));
                ioreq->iouh_Req.io_Error = UHIOERR_CRCERROR;
                retire = TRUE;
                break;

            case (OTCF_CC_WRONGPID>>OTCS_COMPLETIONCODE):
                KPRINTF(200, ("Illegal PID!\n"));
                ioreq->iouh_Req.io_Error = UHIOERR_CRCERROR;
                retire = TRUE;
                break;

            case (OTCF_CC_OVERFLOW>>OTCS_COMPLETIONCODE):
                KPRINTF(200, ("Overflow Error!\n"));
                ioreq->iouh_Req.io_Error = UHIOERR_OVERFLOW;
                retire = TRUE;
                break;

            case (OTCF_CC_SHORTPKT>>OTCS_COMPLETIONCODE):
                KPRINTF(10, ("Short packet %ld < %ld\n", len, otd->otd_Length));
                if((!ioreq->iouh_Req.io_Error) && (!(ioreq->iouh_Flags & UHFF_ALLOWRUNTPKTS)))
                {
                    ioreq->iouh_Req.io_Error = UHIOERR_RUNTPACKET;
                }
                retire = TRUE;
                break;

            case (OTCF_CC_OVERRUN>>OTCS_COMPLETIONCODE):
                KPRINTF(200, ("Data Overrun Error!\n"));
                ioreq->iouh_Req.io_Error = UHIOERR_HOSTERROR;
                retire = TRUE;
                break;

            case (OTCF_CC_UNDERRUN>>OTCS_COMPLETIONCODE):
                KPRINTF(200, ("Data Underrun Error!\n"));
                ioreq->iouh_Req.io_Error = UHIOERR_HOSTERROR;
                retire = TRUE;
                break;

            case (OTCF_CC_INVALID>>OTCS_COMPLETIONCODE):
                KPRINTF(200, ("Not touched?!?\n"));
                break;
        }
        if(READMEM32_LE(&oed->oed_HeadPtr) & OEHF_HALTED)
        {
            KPRINTF(100, ("OED halted!\n"));
            retire = TRUE;
        }

        if(retire)
        {
            Remove(&ioreq->iouh_Req.io_Message.mn_Node);
            AddHead(&hc->hc_OhciRetireQueue, &ioreq->iouh_Req.io_Message.mn_Node);
        }

        if(!otd->otd_NextTD)
        {
            break;
        }
        KPRINTF(1, ("NextTD=%08lx\n", otd->otd_NextTD));
        otd = (struct OhciTD *) ((IPTR)(READMEM32_LE(&otd->otd_NextTD) & OHCI_PTRMASK) - hc->hc_PCIVirtualAdjust - 16);
        KPRINTF(1, ("NextOTD = %08lx\n", otd));
    } while(TRUE);

    ioreq = (struct IOUsbHWReq *) hc->hc_OhciRetireQueue.lh_Head;
    while((nextioreq = (struct IOUsbHWReq *) ((struct Node *) ioreq)->ln_Succ))
    {
        Remove(&ioreq->iouh_Req.io_Message.mn_Node);
        oed = (struct OhciED *) ioreq->iouh_DriverPrivate1;
        if(oed)
        {
            KPRINTF(10, ("Retiring IOReq=%08lx ED=%08lx, Frame=%ld\n", ioreq, oed, READREG32_LE(hc->hc_RegBase, OHCI_FRAMECOUNT)));

            if(oed->oed_Continue)
            {
                ULONG actual = ioreq->iouh_Actual;
                ULONG oldenables;
                ULONG phyaddr;
                struct OhciTD *predotd = NULL;

                KPRINTF(10, ("Reloading Bulk transfer at %ld of %ld\n", ioreq->iouh_Actual, ioreq->iouh_Length));
                otd = oed->oed_FirstTD;
                
                /* 64 FIXME: iouh_Data can lie outside of 32-bit memory. Handle this! */
                phyaddr = (ULONG) pciGetPhysical(hc, &(((UBYTE *) ioreq->iouh_Data)[actual]));
                do
                {
                    len = ioreq->iouh_Length - actual;
                    if(len > OHCI_PAGE_SIZE)
                    {
                        len = OHCI_PAGE_SIZE;
                    }
                    if((!otd->otd_Succ) && (actual + len == ioreq->iouh_Length) && (!(ioreq->iouh_Flags & UHFF_NOSHORTPKT)) && ((actual % ioreq->iouh_MaxPktSize) == 0))
                    {
                        // special case -- zero padding would not fit in this run,
                        // and next time, we would forget about it. So rather abort
                        // reload now, so the zero padding goes with the next reload
                        break;
                    }
                    predotd = otd;
                    otd->otd_Length = len;
                    KPRINTF(1, ("TD with %ld bytes: %08x-%08x\n", len, phyaddr, phyaddr+len-1));
                    CONSTWRITEMEM32_LE(&otd->otd_Ctrl, OTCF_CC_INVALID|OTCF_NOINT);
                    if(otd->otd_Succ)
                    {
                        otd->otd_NextTD = otd->otd_Succ->otd_Self;
                    }
                    if(len)
                    {
                        WRITEMEM32_LE(&otd->otd_BufferPtr, CachePreDMA((APTR)(IPTR)phyaddr, &len, (ioreq->iouh_Dir == UHDIR_IN) ? 0 : DMA_ReadFromRAM));
                        phyaddr += len - 1;
                        WRITEMEM32_LE(&otd->otd_BufferEnd, phyaddr);
                        phyaddr++;
                    } else {
                        CONSTWRITEMEM32_LE(&otd->otd_BufferPtr, 0);
                        CONSTWRITEMEM32_LE(&otd->otd_BufferEnd, 0);
                    }
                    CacheClearE(&otd->otd_Ctrl, 16, CACRF_ClearD);
                    actual += len;
                    otd = otd->otd_Succ;
                } while(otd && ((actual < ioreq->iouh_Length) || (len && (ioreq->iouh_Dir == UHDIR_OUT) && (actual == ioreq->iouh_Length) && (!(ioreq->iouh_Flags & UHFF_NOSHORTPKT)) && ((actual % ioreq->iouh_MaxPktSize) == 0))));
                oed->oed_Continue = (actual < ioreq->iouh_Length);
                predotd->otd_NextTD = hc->hc_OhciTermTD->otd_Self;

                CONSTWRITEMEM32_LE(&predotd->otd_Ctrl, OTCF_CC_INVALID);
                CacheClearE(&predotd->otd_Ctrl, 16, CACRF_ClearD);

                Disable();
                AddTail(&hc->hc_TDQueue, &ioreq->iouh_Req.io_Message.mn_Node);

                // keep toggle bit
                ctrlstatus = READMEM32_LE(&oed->oed_HeadPtr) & OEHF_DATA1;
                WRITEMEM32_LE(&oed->oed_HeadPtr, READMEM32_LE(&oed->oed_FirstTD->otd_Self)|ctrlstatus);
                CacheClearE(&oed->oed_EPCaps, 16, CACRF_ClearD);

                oldenables = READREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS);
                oldenables |= OCSF_BULKENABLE;
                WRITEREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS, oldenables);
                SYNC;
                Enable();
            } else {
                // disable ED
                ctrlstatus = READMEM32_LE(&oed->oed_HeadPtr);
                ctrlstatus |= OEHF_HALTED;
                WRITEMEM32_LE(&oed->oed_HeadPtr, ctrlstatus);

                devadrep = (ioreq->iouh_DevAddr<<5) + ioreq->iouh_Endpoint + ((ioreq->iouh_Dir == UHDIR_IN) ? 0x10 : 0);
                unit->hu_DevBusyReq[devadrep] = NULL;
                unit->hu_DevDataToggle[devadrep] = (ctrlstatus & OEHF_DATA1) ? TRUE : FALSE;

                ohciFreeEDContext(hc, oed);
                if(ioreq->iouh_Req.io_Command == UHCMD_INTXFER)
                {
                    updatetree = TRUE;
                }
                // check for successful clear feature and set address ctrl transfers
                if((!ioreq->iouh_Req.io_Error) && (ioreq->iouh_Req.io_Command == UHCMD_CONTROLXFER))
                {
                    uhwCheckSpecialCtrlTransfers(hc, ioreq);
                }
                ReplyMsg(&ioreq->iouh_Req.io_Message);
            }
        } else {
            KPRINTF(20, ("IOReq=%08lx has no OED!\n", ioreq));
        }
        ioreq = nextioreq;
    }
    if(updatetree)
    {
        ohciUpdateIntTree(hc);
    }
}

void ohciScheduleCtrlTDs(struct PCIController *hc) {

    struct PCIUnit *unit = hc->hc_Unit;
    struct IOUsbHWReq *ioreq;
    UWORD devadrep;
    struct OhciED *oed;
    struct OhciTD *setupotd;
    struct OhciTD *dataotd;
    struct OhciTD *termotd;
    struct OhciTD *predotd;
    ULONG actual;
    ULONG epcaps;
    ULONG ctrl;
    ULONG len;
    ULONG phyaddr;
    ULONG oldenables;

    /* *** CTRL Transfers *** */
    KPRINTF(1, ("Scheduling new CTRL transfers...\n"));
    ioreq = (struct IOUsbHWReq *) hc->hc_CtrlXFerQueue.lh_Head;
    while(((struct Node *) ioreq)->ln_Succ)
    {
        devadrep = (ioreq->iouh_DevAddr<<5) + ioreq->iouh_Endpoint;
        KPRINTF(10, ("New CTRL transfer to %ld.%ld: %ld bytes\n", ioreq->iouh_DevAddr, ioreq->iouh_Endpoint, ioreq->iouh_Length));
        /* is endpoint already in use or do we have to wait for next transaction */
        if(unit->hu_DevBusyReq[devadrep])
        {
            KPRINTF(5, ("Endpoint %02lx in use!\n", devadrep));
            ioreq = (struct IOUsbHWReq *) ((struct Node *) ioreq)->ln_Succ;
            continue;
        }

        oed = ohciAllocED(hc);
        if(!oed)
        {
            break;
        }

        setupotd = ohciAllocTD(hc);
        if(!setupotd)
        {
            ohciFreeED(hc, oed);
            break;
        }
        termotd = ohciAllocTD(hc);
        if(!termotd)
        {
            ohciFreeTD(hc, setupotd);
            ohciFreeED(hc, oed);
            break;
        }
        oed->oed_IOReq = ioreq;

        KPRINTF(1, ("SetupTD=%08lx, TermTD=%08lx\n", setupotd, termotd));

        // fill setup td
        epcaps = (ioreq->iouh_DevAddr<<OECS_DEVADDR)|(ioreq->iouh_Endpoint<<OECS_ENDPOINT)|(ioreq->iouh_MaxPktSize<<OECS_MAXPKTLEN)|OECF_DIRECTION_TD;

        if(ioreq->iouh_Flags & UHFF_LOWSPEED)
        {
            KPRINTF(5, ("*** LOW SPEED ***\n"));
            epcaps |= OECF_LOWSPEED;
        }

        WRITEMEM32_LE(&oed->oed_EPCaps, epcaps);

        oed->oed_TailPtr = hc->hc_OhciTermTD->otd_Self;
        oed->oed_HeadPtr = setupotd->otd_Self;
        oed->oed_FirstTD = setupotd;

        setupotd->otd_ED = oed;
        setupotd->otd_Length = 0; // don't increase io_Actual for that transfer
        CONSTWRITEMEM32_LE(&setupotd->otd_Ctrl, OTCF_PIDCODE_SETUP|OTCF_CC_INVALID|OTCF_NOINT);
        len = 8;
        
        /* 64 FIXME: Handle SetupData which is outside of 32-bit memory */
        WRITEMEM32_LE(&setupotd->otd_BufferPtr, (ULONG) CachePreDMA(pciGetPhysical(hc, &ioreq->iouh_SetupData), &len, DMA_ReadFromRAM));
        WRITEMEM32_LE(&setupotd->otd_BufferEnd, (ULONG) pciGetPhysical(hc, ((UBYTE *) (&ioreq->iouh_SetupData)) + 7));

        KPRINTF(1, ("TD send: %08x - %08x\n", READMEM32_LE(&setupotd->otd_BufferPtr),
        		READMEM32_LE(&setupotd->otd_BufferEnd)));

        ctrl = (ioreq->iouh_SetupData.bmRequestType & URTF_IN) ? (OTCF_PIDCODE_IN|OTCF_CC_INVALID|OTCF_NOINT) : (OTCF_PIDCODE_OUT|OTCF_CC_INVALID|OTCF_NOINT);

        predotd = setupotd;
        if(ioreq->iouh_Length)
        {
            /* 64 FIXME: Handle iouh_Data outside of 32-bit space */
            phyaddr = (ULONG) pciGetPhysical(hc, ioreq->iouh_Data);
            actual = 0;
            do
            {
                dataotd = ohciAllocTD(hc);
                if(!dataotd)
                {
                    predotd->otd_Succ = NULL;
                    break;
                }
                dataotd->otd_ED = oed;
                predotd->otd_Succ = dataotd;
                predotd->otd_NextTD = dataotd->otd_Self;
                len = ioreq->iouh_Length - actual;
                if(len > OHCI_PAGE_SIZE)
                {
                    len = OHCI_PAGE_SIZE;
                }
                dataotd->otd_Length = len;
                KPRINTF(1, ("TD with %ld bytes\n", len));
                WRITEMEM32_LE(&dataotd->otd_Ctrl, ctrl);
                WRITEMEM32_LE(&dataotd->otd_BufferPtr, CachePreDMA((APTR)(IPTR)phyaddr, &len, (ioreq->iouh_SetupData.bmRequestType & URTF_IN) ? 0 : DMA_ReadFromRAM));
                phyaddr += len - 1;
                WRITEMEM32_LE(&dataotd->otd_BufferEnd, phyaddr);

                KPRINTF(1, ("TD send: %08x - %08x\n", READMEM32_LE(&dataotd->otd_BufferPtr),
                		READMEM32_LE(&dataotd->otd_BufferEnd)));

                CacheClearE(&dataotd->otd_Ctrl, 16, CACRF_ClearD);
                phyaddr++;
                actual += len;
                predotd = dataotd;
            } while(actual < ioreq->iouh_Length);

            if(actual != ioreq->iouh_Length)
            {
                // out of TDs
                KPRINTF(200, ("Out of TDs for Ctrl Transfer!\n"));
                dataotd = setupotd->otd_Succ;
                ohciFreeTD(hc, setupotd);
                while(dataotd)
                {
                    predotd = dataotd;
                    dataotd = dataotd->otd_Succ;
                    ohciFreeTD(hc, predotd);
                }
                ohciFreeTD(hc, termotd);
                ohciFreeED(hc, oed);
                break;
            }
            predotd->otd_Succ = termotd;
            predotd->otd_NextTD = termotd->otd_Self;
        } else {
            setupotd->otd_Succ = termotd;
            setupotd->otd_NextTD = termotd->otd_Self;
        }
        CacheClearE(&setupotd->otd_Ctrl, 16, CACRF_ClearD);
        CacheClearE(&predotd->otd_Ctrl, 16, CACRF_ClearD);

        ctrl ^= (OTCF_PIDCODE_IN^OTCF_PIDCODE_OUT)|OTCF_NOINT|OTCF_DATA1|OTCF_TOGGLEFROMTD;

        termotd->otd_Length = 0;
        termotd->otd_ED = oed;
        termotd->otd_Succ = NULL;
        termotd->otd_NextTD = hc->hc_OhciTermTD->otd_Self;
        CONSTWRITEMEM32_LE(&termotd->otd_Ctrl, ctrl);
        CONSTWRITEMEM32_LE(&termotd->otd_BufferPtr, 0);
        CONSTWRITEMEM32_LE(&termotd->otd_BufferEnd, 0);
        CacheClearE(&termotd->otd_Ctrl, 16, CACRF_ClearD);

        Remove(&ioreq->iouh_Req.io_Message.mn_Node);
        ioreq->iouh_DriverPrivate1 = oed;

        // manage endpoint going busy
        unit->hu_DevBusyReq[devadrep] = ioreq;
        unit->hu_NakTimeoutFrame[devadrep] = (ioreq->iouh_Flags & UHFF_NAKTIMEOUT) ? hc->hc_FrameCounter + ioreq->iouh_NakTimeout : 0;

        Disable();
        AddTail(&hc->hc_TDQueue, (struct Node *) ioreq);

        // looks good to me, now enqueue this entry
        oed->oed_Succ = hc->hc_OhciCtrlTailED;
        oed->oed_NextED = oed->oed_Succ->oed_Self;
        oed->oed_Pred = hc->hc_OhciCtrlTailED->oed_Pred;
        CacheClearE(&oed->oed_Pred->oed_EPCaps, 16, CACRF_InvalidateD);
        oed->oed_Pred->oed_Succ = oed;
        oed->oed_Pred->oed_NextED = oed->oed_Self;
        oed->oed_Succ->oed_Pred = oed;
        CacheClearE(&oed->oed_EPCaps, 16, CACRF_ClearD);
        CacheClearE(&oed->oed_Pred->oed_EPCaps, 16, CACRF_ClearD);
        SYNC;

        KPRINTF(5, ("ED: EPCaps=%08lx, HeadPtr=%08lx, TailPtr=%08lx, NextED=%08lx\n",
                     READMEM32_LE(&oed->oed_EPCaps),
                     READMEM32_LE(&oed->oed_HeadPtr),
                     READMEM32_LE(&oed->oed_TailPtr),
                     READMEM32_LE(&oed->oed_NextED)));

        oldenables = READREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS);
        if(!(oldenables & OCSF_CTRLENABLE))
        {
            CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_CTRL_ED, 0);
        }
        oldenables |= OCSF_CTRLENABLE;
        WRITEREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS, oldenables);
        SYNC;
        Enable();

        ioreq = (struct IOUsbHWReq *) hc->hc_CtrlXFerQueue.lh_Head;
    }
}

void ohciScheduleIntTDs(struct PCIController *hc) {

    struct PCIUnit *unit = hc->hc_Unit;
    struct IOUsbHWReq *ioreq;
    UWORD devadrep;
    struct OhciED *intoed;
    struct OhciED *oed;
    struct OhciTD *otd;
    struct OhciTD *predotd;
    ULONG actual;
    ULONG epcaps;
    ULONG len;
    ULONG phyaddr;

    /* *** INT Transfers *** */
    KPRINTF(1, ("Scheduling new INT transfers...\n"));
    ioreq = (struct IOUsbHWReq *) hc->hc_IntXFerQueue.lh_Head;
    while(((struct Node *) ioreq)->ln_Succ)
    {
        devadrep = (ioreq->iouh_DevAddr<<5) + ioreq->iouh_Endpoint + ((ioreq->iouh_Dir == UHDIR_IN) ? 0x10 : 0);
        KPRINTF(10, ("New INT transfer to %ld.%ld: %ld bytes\n", ioreq->iouh_DevAddr, ioreq->iouh_Endpoint, ioreq->iouh_Length));
        /* is endpoint already in use or do we have to wait for next transaction */
        if(unit->hu_DevBusyReq[devadrep])
        {
            KPRINTF(5, ("Endpoint %02lx in use!\n", devadrep));
            ioreq = (struct IOUsbHWReq *) ((struct Node *) ioreq)->ln_Succ;
            continue;
        }

        oed = ohciAllocED(hc);
        if(!oed)
        {
            break;
        }

        oed->oed_IOReq = ioreq;

        epcaps = (ioreq->iouh_DevAddr<<OECS_DEVADDR)|(ioreq->iouh_Endpoint<<OECS_ENDPOINT)|(ioreq->iouh_MaxPktSize<<OECS_MAXPKTLEN);
        epcaps |= (ioreq->iouh_Dir == UHDIR_IN) ? OECF_DIRECTION_IN : OECF_DIRECTION_OUT;

        if(ioreq->iouh_Flags & UHFF_LOWSPEED)
        {
            KPRINTF(5, ("*** LOW SPEED ***\n"));
            epcaps |= OECF_LOWSPEED;
        }

        WRITEMEM32_LE(&oed->oed_EPCaps, epcaps);
        oed->oed_TailPtr = hc->hc_OhciTermTD->otd_Self;

        predotd = NULL;
        /* 64 FIXME: 32-bit data */
        phyaddr = (ULONG) pciGetPhysical(hc, ioreq->iouh_Data);
        actual = 0;
        do
        {
            otd = ohciAllocTD(hc);
            if(!otd)
            {
                predotd->otd_Succ = NULL;
                break;
            }
            otd->otd_ED = oed;
            if(predotd)
            {
                predotd->otd_Succ = otd;
                predotd->otd_NextTD = otd->otd_Self;
            } else {
                WRITEMEM32_LE(&oed->oed_HeadPtr, READMEM32_LE(&otd->otd_Self)|(unit->hu_DevDataToggle[devadrep] ? OEHF_DATA1 : 0));
                oed->oed_FirstTD = otd;
            }
            len = ioreq->iouh_Length - actual;
            if(len > OHCI_PAGE_SIZE)
            {
                len = OHCI_PAGE_SIZE;
            }
            otd->otd_Length = len;
            KPRINTF(1, ("TD with %ld bytes\n", len));
            CONSTWRITEMEM32_LE(&otd->otd_Ctrl, OTCF_CC_INVALID|OTCF_NOINT);
            if(len)
            {
                WRITEMEM32_LE(&otd->otd_BufferPtr, CachePreDMA((APTR)(IPTR)phyaddr, &len, (ioreq->iouh_Dir == UHDIR_IN) ? 0 : DMA_ReadFromRAM));
                phyaddr += len - 1;
                WRITEMEM32_LE(&otd->otd_BufferEnd, phyaddr);
                phyaddr++;
            } else {
                CONSTWRITEMEM32_LE(&otd->otd_BufferPtr, 0);
                CONSTWRITEMEM32_LE(&otd->otd_BufferEnd, 0);
            }
            actual += len;
            CacheClearE(&otd->otd_Ctrl, 16, CACRF_ClearD);
            predotd = otd;
        } while(actual < ioreq->iouh_Length);

        if(actual != ioreq->iouh_Length)
        {
            // out of TDs
            KPRINTF(200, ("Out of TDs for Int Transfer!\n"));
            otd = oed->oed_FirstTD;
            while(otd)
            {
                predotd = otd;
                otd = otd->otd_Succ;
                ohciFreeTD(hc, predotd);
            }
            ohciFreeED(hc, oed);
            break;
        }
        predotd->otd_Succ = NULL;
        predotd->otd_NextTD = hc->hc_OhciTermTD->otd_Self;

        CONSTWRITEMEM32_LE(&predotd->otd_Ctrl, OTCF_CC_INVALID);
        CacheClearE(&predotd->otd_Ctrl, 16, CACRF_ClearD);

        if(ioreq->iouh_Interval >= 31)
        {
            intoed = hc->hc_OhciIntED[4]; // 32ms interval
        } else {
            UWORD cnt = 0;
            do
            {
                intoed = hc->hc_OhciIntED[cnt++];
            } while(ioreq->iouh_Interval >= (1<<cnt));
        }

        Remove(&ioreq->iouh_Req.io_Message.mn_Node);
        ioreq->iouh_DriverPrivate1 = oed;

        // manage endpoint going busy
        unit->hu_DevBusyReq[devadrep] = ioreq;
        unit->hu_NakTimeoutFrame[devadrep] = (ioreq->iouh_Flags & UHFF_NAKTIMEOUT) ? hc->hc_FrameCounter + ioreq->iouh_NakTimeout : 0;

        Disable();
        AddTail(&hc->hc_TDQueue, (struct Node *) ioreq);

        // looks good to me, now enqueue this entry (behind Int head)
        oed->oed_Succ = intoed->oed_Succ;
        oed->oed_NextED = intoed->oed_Succ->oed_Self;
        oed->oed_Pred = intoed;
        intoed->oed_Succ = oed;
        intoed->oed_NextED = oed->oed_Self;
        oed->oed_Succ->oed_Pred = oed;
        CacheClearE(&oed->oed_EPCaps, 16, CACRF_ClearD);
        CacheClearE(&intoed->oed_EPCaps, 16, CACRF_ClearD);
        SYNC;

        KPRINTF(5, ("ED: EPCaps=%08lx, HeadPtr=%08lx, TailPtr=%08lx, NextED=%08lx\n",
                     READMEM32_LE(&oed->oed_EPCaps),
                     READMEM32_LE(&oed->oed_HeadPtr),
                     READMEM32_LE(&oed->oed_TailPtr),
                     READMEM32_LE(&oed->oed_NextED)));
        Enable();

        ioreq = (struct IOUsbHWReq *) hc->hc_IntXFerQueue.lh_Head;
    }
}

void ohciScheduleBulkTDs(struct PCIController *hc) {

    struct PCIUnit *unit = hc->hc_Unit;
    struct IOUsbHWReq *ioreq;
    UWORD devadrep;
    struct OhciED *oed;
    struct OhciTD *otd;
    struct OhciTD *predotd;
    ULONG actual;
    ULONG epcaps;
    ULONG len;
    ULONG phyaddr;
    ULONG oldenables;

    /* *** BULK Transfers *** */
    KPRINTF(1, ("Scheduling new BULK transfers...\n"));
    ioreq = (struct IOUsbHWReq *) hc->hc_BulkXFerQueue.lh_Head;
    while(((struct Node *) ioreq)->ln_Succ)
    {
        devadrep = (ioreq->iouh_DevAddr<<5) + ioreq->iouh_Endpoint + ((ioreq->iouh_Dir == UHDIR_IN) ? 0x10 : 0);
        KPRINTF(10, ("New BULK transfer to %ld.%ld: %ld bytes\n", ioreq->iouh_DevAddr, ioreq->iouh_Endpoint, ioreq->iouh_Length));
        /* is endpoint already in use or do we have to wait for next transaction */
        if(unit->hu_DevBusyReq[devadrep])
        {
            KPRINTF(5, ("Endpoint %02lx in use!\n", devadrep));
            ioreq = (struct IOUsbHWReq *) ((struct Node *) ioreq)->ln_Succ;
            continue;
        }

        oed = ohciAllocED(hc);
        if(!oed)
        {
            break;
        }

        oed->oed_IOReq = ioreq;

        epcaps = (ioreq->iouh_DevAddr<<OECS_DEVADDR)|(ioreq->iouh_Endpoint<<OECS_ENDPOINT)|(ioreq->iouh_MaxPktSize<<OECS_MAXPKTLEN);
        epcaps |= (ioreq->iouh_Dir == UHDIR_IN) ? OECF_DIRECTION_IN : OECF_DIRECTION_OUT;

        if(ioreq->iouh_Flags & UHFF_LOWSPEED)
        {
            KPRINTF(5, ("*** LOW SPEED ***\n"));
            epcaps |= OECF_LOWSPEED;
        }

        WRITEMEM32_LE(&oed->oed_EPCaps, epcaps);
        oed->oed_TailPtr = hc->hc_OhciTermTD->otd_Self;

        predotd = NULL;
        /* 64 FIXME */
        phyaddr = (ULONG) pciGetPhysical(hc, ioreq->iouh_Data);
        actual = 0;
        do
        {
            if((actual >= OHCI_TD_BULK_LIMIT) && (actual < ioreq->iouh_Length))
            {
                KPRINTF(10, ("Bulk too large, splitting...\n"));
                break;
            }
            otd = ohciAllocTD(hc);
            if(!otd)
            {
                predotd->otd_Succ = NULL;
                break;
            }
            otd->otd_ED = oed;
            if(predotd)
            {
                predotd->otd_Succ = otd;
                predotd->otd_NextTD = otd->otd_Self;
            } else {
                WRITEMEM32_LE(&oed->oed_HeadPtr, READMEM32_LE(&otd->otd_Self)|(unit->hu_DevDataToggle[devadrep] ? OEHF_DATA1 : 0));
                oed->oed_FirstTD = otd;
            }
            len = ioreq->iouh_Length - actual;
            if(len > OHCI_PAGE_SIZE)
            {
                len = OHCI_PAGE_SIZE;
            }
            otd->otd_Length = len;
            KPRINTF(1, ("TD with %ld bytes: %08x-%08x\n", len, phyaddr, phyaddr+len-1));
            CONSTWRITEMEM32_LE(&otd->otd_Ctrl, OTCF_CC_INVALID|OTCF_NOINT);
            if(len)
            {
                WRITEMEM32_LE(&otd->otd_BufferPtr, CachePreDMA((APTR)(IPTR)phyaddr, &len, (ioreq->iouh_Dir == UHDIR_IN) ? 0 : DMA_ReadFromRAM));
                phyaddr += len - 1;
                WRITEMEM32_LE(&otd->otd_BufferEnd, phyaddr);
                phyaddr++;
            } else {
                CONSTWRITEMEM32_LE(&otd->otd_BufferPtr, 0);
                CONSTWRITEMEM32_LE(&otd->otd_BufferEnd, 0);
            }
            actual += len;
            CacheClearE(&otd->otd_Ctrl, 16, CACRF_ClearD);

            predotd = otd;
        } while((actual < ioreq->iouh_Length) || (len && (ioreq->iouh_Dir == UHDIR_OUT) && (actual == ioreq->iouh_Length) && (!(ioreq->iouh_Flags & UHFF_NOSHORTPKT)) && ((actual % ioreq->iouh_MaxPktSize) == 0)));

        if(!actual)
        {
            // out of TDs
            KPRINTF(200, ("Out of TDs for Bulk Transfer!\n"));
            otd = oed->oed_FirstTD;
            while(otd)
            {
                predotd = otd;
                otd = otd->otd_Succ;
                ohciFreeTD(hc, predotd);
            }
            ohciFreeED(hc, oed);
            break;
        }
        oed->oed_Continue = (actual < ioreq->iouh_Length);
        predotd->otd_Succ = NULL;
        predotd->otd_NextTD = hc->hc_OhciTermTD->otd_Self;

        CONSTWRITEMEM32_LE(&predotd->otd_Ctrl, OTCF_CC_INVALID);
        CacheClearE(&predotd->otd_Ctrl, 16, CACRF_ClearD);

        Remove(&ioreq->iouh_Req.io_Message.mn_Node);
        ioreq->iouh_DriverPrivate1 = oed;

        // manage endpoint going busy
        unit->hu_DevBusyReq[devadrep] = ioreq;
        unit->hu_NakTimeoutFrame[devadrep] = (ioreq->iouh_Flags & UHFF_NAKTIMEOUT) ? hc->hc_FrameCounter + ioreq->iouh_NakTimeout : 0;

        Disable();
        AddTail(&hc->hc_TDQueue, (struct Node *) ioreq);

        // looks good to me, now enqueue this entry
        oed->oed_Succ = hc->hc_OhciBulkTailED;
        oed->oed_NextED = oed->oed_Succ->oed_Self;
        oed->oed_Pred = hc->hc_OhciBulkTailED->oed_Pred;
        oed->oed_Pred->oed_Succ = oed;
        oed->oed_Pred->oed_NextED = oed->oed_Self;
        oed->oed_Succ->oed_Pred = oed;
        CacheClearE(&oed->oed_EPCaps, 16, CACRF_ClearD);
        CacheClearE(&oed->oed_Pred->oed_EPCaps, 16, CACRF_ClearD);
        SYNC;

        KPRINTF(10, ("Activating BULK at %ld\n", READREG32_LE(hc->hc_RegBase, OHCI_FRAMECOUNT)));

        KPRINTF(5, ("ED: EPCaps=%08lx, HeadPtr=%08lx, TailPtr=%08lx, NextED=%08lx\n",
                     READMEM32_LE(&oed->oed_EPCaps),
                     READMEM32_LE(&oed->oed_HeadPtr),
                     READMEM32_LE(&oed->oed_TailPtr),
                     READMEM32_LE(&oed->oed_NextED)));

        oldenables = READREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS);
        if(!(oldenables & OCSF_BULKENABLE))
        {
            CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_BULK_ED, 0);
        }
        oldenables |= OCSF_BULKENABLE;
        WRITEREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS, oldenables);
        SYNC;
        Enable();
        ioreq = (struct IOUsbHWReq *) hc->hc_BulkXFerQueue.lh_Head;
    }
}

void ohciUpdateFrameCounter(struct PCIController *hc) {

    Disable();
    hc->hc_FrameCounter = (hc->hc_FrameCounter & 0xffff0000)|(READREG32_LE(hc->hc_RegBase, OHCI_FRAMECOUNT) & 0xffff);
    Enable();
}

void ohciCompleteInt(struct PCIController *hc) {

    KPRINTF(1, ("CompleteInt!\n"));

    ohciUpdateFrameCounter(hc);

    /* **************** PROCESS DONE TRANSFERS **************** */

    if(hc->hc_OhciDoneQueue) {
        ohciHandleFinishedTDs(hc);
    }

    if(hc->hc_CtrlXFerQueue.lh_Head->ln_Succ) {
        ohciScheduleCtrlTDs(hc);
    }

    if(hc->hc_IntXFerQueue.lh_Head->ln_Succ) {
        ohciScheduleIntTDs(hc);
    }

    if(hc->hc_BulkXFerQueue.lh_Head->ln_Succ) {
        ohciScheduleBulkTDs(hc);
    }

    KPRINTF(1, ("CompleteDone\n"));
}

void ohciIntCode(HIDDT_IRQ_Handler *irq, HIDDT_IRQ_HwInfo *hw) {

    struct PCIController *hc = (struct PCIController *) irq->h_Data;
    struct PCIDevice *base = hc->hc_Device;
    struct PCIUnit *unit = hc->hc_Unit;
    ULONG intr = 0;
    ULONG donehead;

    CacheClearE(&hc->hc_OhciHCCA->oha_DoneHead, sizeof(hc->hc_OhciHCCA->oha_DoneHead), CACRF_InvalidateD);

    donehead = READMEM32_LE(&hc->hc_OhciHCCA->oha_DoneHead);

    if(donehead)
    {
    	if (donehead & ~1)
    		intr = OISF_DONEHEAD;
    	if(donehead & 1)
    	{
    		intr |= READREG32_LE(hc->hc_RegBase, OHCI_INTSTATUS);
    	}
    	donehead &= OHCI_PTRMASK;

    	CONSTWRITEMEM32_LE(&hc->hc_OhciHCCA->oha_DoneHead, 0);

    	KPRINTF(5, ("New Donehead %08lx for old %08lx\n", donehead, hc->hc_OhciDoneQueue));
    } else {
    	intr = READREG32_LE(hc->hc_RegBase, OHCI_INTSTATUS);

    	if (intr & OISF_DONEHEAD)
    	{
    		KPRINTF(1, ("!!!!!!!!!!!!!!!!!!!!!!!DoneHead was empty!!!!!!!!!!!!!!!!!!!\n"));
    		CacheClearE(hc->hc_OhciHCCA, sizeof(struct OhciHCCA), CACRF_InvalidateD);
    		donehead = READMEM32_LE(&hc->hc_OhciHCCA->oha_DoneHead) & OHCI_PTRMASK;
    		CONSTWRITEMEM32_LE(&hc->hc_OhciHCCA->oha_DoneHead, 0);

    		KPRINTF(5, ("New Donehead %08lx for old %08lx\n", donehead, hc->hc_OhciDoneQueue));
    	}
    }
    CacheClearE(hc->hc_OhciHCCA, sizeof(struct OhciHCCA), CACRF_ClearD);

    intr &= ~OISF_MASTERENABLE;

    if(intr & hc->hc_PCIIntEnMask)
    {
        WRITEREG32_LE(hc->hc_RegBase, OHCI_INTSTATUS, intr);
        //KPRINTF(1, ("INT=%02lx\n", intr));
        if(intr & OISF_HOSTERROR)
        {
            KPRINTF(200, ("Host ERROR!\n"));
        }
        if(intr & OISF_SCHEDOVERRUN)
        {
            KPRINTF(200, ("Schedule overrun!\n"));
        }
        if(!hc->hc_Online)
        {
            if(READREG32_LE(hc->hc_RegBase, OHCI_INTSTATUS) & OISF_HUBCHANGE)
            {
                // if the driver is not online and the controller has a broken
                // hub change interrupt, make sure we don't run into infinite
                // interrupt by disabling the interrupt bit
                WRITEREG32_LE(hc->hc_RegBase, OHCI_INTDIS, OISF_HUBCHANGE);
            }
            return;
        }
        WRITEREG32_LE(hc->hc_RegBase, OHCI_INTEN, OISF_HUBCHANGE);
        if(intr & OISF_FRAMECOUNTOVER)
        {
            hc->hc_FrameCounter |= 0x7fff;
            hc->hc_FrameCounter++;
            hc->hc_FrameCounter |= READREG32_LE(hc->hc_RegBase, OHCI_FRAMECOUNT) & 0xffff;
            KPRINTF(10, ("Frame Counter Rollover %ld\n", hc->hc_FrameCounter));
        }
        if(intr & OISF_HUBCHANGE)
        {
            UWORD hciport;
            ULONG oldval;
            UWORD portreg = OHCI_PORTSTATUS;
            BOOL clearbits = FALSE;

            if(READREG32_LE(hc->hc_RegBase, OHCI_INTSTATUS) & OISF_HUBCHANGE)
            {
                // some OHCI implementations will keep the interrupt bit stuck until
                // all port changes have been cleared, which is wrong according to the
                // OHCI spec. As a workaround we will clear all change bits, which should
                // be no problem as the port changes are reflected in the PortChangeMap
                // array.
                clearbits = TRUE;
            }
            for(hciport = 0; hciport < hc->hc_NumPorts; hciport++, portreg += 4)
            {
                oldval = READREG32_LE(hc->hc_RegBase, portreg);
                if(oldval & OHPF_OVERCURRENTCHG)
                {
                    hc->hc_PortChangeMap[hciport] |= UPSF_PORT_OVER_CURRENT;
                }
                if(oldval & OHPF_RESETCHANGE)
                {
                    hc->hc_PortChangeMap[hciport] |= UPSF_PORT_RESET;
                }
                if(oldval & OHPF_ENABLECHANGE)
                {
                    hc->hc_PortChangeMap[hciport] |= UPSF_PORT_ENABLE;
                }
                if(oldval & OHPF_CONNECTCHANGE)
                {
                    hc->hc_PortChangeMap[hciport] |= UPSF_PORT_CONNECTION;
                }
                if(oldval & OHPF_RESUMEDTX)
                {
                    hc->hc_PortChangeMap[hciport] |= UPSF_PORT_SUSPEND;
                }
                if(clearbits)
                {
                    WRITEREG32_LE(hc->hc_RegBase, portreg, OHPF_CONNECTCHANGE|OHPF_ENABLECHANGE|OHPF_RESUMEDTX|OHPF_OVERCURRENTCHG|OHPF_RESETCHANGE);
                }

                KPRINTF(20, ("PCI Int Port %ld (glob %ld) Change %08lx\n", hciport, hc->hc_PortNum20[hciport] + 1, oldval));
                if(hc->hc_PortChangeMap[hciport])
                {
                    unit->hu_RootPortChanges |= 1UL<<(hc->hc_PortNum20[hciport] + 1);
                }
            }
            uhwCheckRootHubChanges(unit);
            if(clearbits)
            {
                // again try to get rid of any bits that may be causing the interrupt
                WRITEREG32_LE(hc->hc_RegBase, OHCI_HUBSTATUS, OHSF_OVERCURRENTCHG);
                WRITEREG32_LE(hc->hc_RegBase, OHCI_INTSTATUS, OISF_HUBCHANGE);
            }
        }
        if(intr & OISF_DONEHEAD)
        {
        	KPRINTF(10, ("DoneHead %ld\n", READREG32_LE(hc->hc_RegBase, OHCI_FRAMECOUNT)));

        	if(hc->hc_OhciDoneQueue)
        	{
        		struct OhciTD *donetd = (struct OhciTD *) ((IPTR)donehead - hc->hc_PCIVirtualAdjust - 16);

        		CacheClearE(&donetd->otd_Ctrl, 16, CACRF_InvalidateD);
        		while(donetd->otd_NextTD)
        		{
        			donetd = (struct OhciTD *) ((IPTR)donetd->otd_NextTD - hc->hc_PCIVirtualAdjust - 16);
        			CacheClearE(&donetd->otd_Ctrl, 16, CACRF_InvalidateD);
        		}
        		WRITEMEM32_LE(&donetd->otd_NextTD, hc->hc_OhciDoneQueue);
        		CacheClearE(&donetd->otd_Ctrl, 16, CACRF_ClearD);
        	}
        	hc->hc_OhciDoneQueue = donehead;

        	SureCause(base, &hc->hc_CompleteInt);
        }
    }

    /* Unlock interrupts  */
    WRITEREG32_LE(&hc->hc_RegBase, OHCI_INTEN, OISF_MASTERENABLE);
}

BOOL ohciInit(struct PCIController *hc, struct PCIUnit *hu) {

    struct PCIDevice *hd = hu->hu_Device;

    struct OhciED *oed;
    struct OhciED *predoed;
    struct OhciTD *otd;
    ULONG *tabptr;
    UBYTE *memptr;
    ULONG bitcnt;
    ULONG hubdesca;
    ULONG cmdstatus;
    ULONG control;
    ULONG timeout;
    ULONG frameival;

    ULONG cnt;

    struct TagItem pciActivateMem[] =
    {
            { aHidd_PCIDevice_isMEM,    TRUE },
            { TAG_DONE, 0UL },
    };

    struct TagItem pciActivateBusmaster[] =
    {
            { aHidd_PCIDevice_isMaster, TRUE },
            { TAG_DONE, 0UL },
    };

    struct TagItem pciDeactivateBusmaster[] =
    {
            { aHidd_PCIDevice_isMaster, FALSE },
            { TAG_DONE, 0UL },
    };

    hc->hc_CompleteInt.is_Node.ln_Type = NT_INTERRUPT;
    hc->hc_CompleteInt.is_Node.ln_Name = "OHCI CompleteInt";
    hc->hc_CompleteInt.is_Node.ln_Pri  = 0;
    hc->hc_CompleteInt.is_Data = hc;
    hc->hc_CompleteInt.is_Code = (void (*)(void)) &ohciCompleteInt;

    hc->hc_PCIMemSize = OHCI_HCCA_SIZE + OHCI_HCCA_ALIGNMENT + 1;
    hc->hc_PCIMemSize += sizeof(struct OhciED) * OHCI_ED_POOLSIZE;
    hc->hc_PCIMemSize += sizeof(struct OhciTD) * OHCI_TD_POOLSIZE;

    memptr = HIDD_PCIDriver_AllocPCIMem(hc->hc_PCIDriverObject, hc->hc_PCIMemSize);
    hc->hc_PCIMem = (APTR) memptr;
    if (memptr)
    {
        // PhysicalAddress - VirtualAdjust = VirtualAddress
        // VirtualAddress  + VirtualAdjust = PhysicalAddress
        hc->hc_PCIVirtualAdjust = pciGetPhysical(hc, memptr) - (APTR)memptr;
        KPRINTF(10, ("VirtualAdjust 0x%08lx\n", hc->hc_PCIVirtualAdjust));

        // align memory
        memptr = (UBYTE *) (((IPTR)hc->hc_PCIMem + OHCI_HCCA_ALIGNMENT) & (~OHCI_HCCA_ALIGNMENT));
        hc->hc_OhciHCCA = (struct OhciHCCA *) memptr;
        KPRINTF(10, ("HCCA 0x%08lx\n", hc->hc_OhciHCCA));
        memptr += OHCI_HCCA_SIZE;

        // build up ED pool
        oed = (struct OhciED *) memptr;
        hc->hc_OhciEDPool = oed;
        cnt = OHCI_ED_POOLSIZE - 1;
        do {
            // minimal initalization
            oed->oed_Succ = (oed + 1);
            WRITEMEM32_LE(&oed->oed_Self, (IPTR)(&oed->oed_EPCaps) + hc->hc_PCIVirtualAdjust);
            oed++;
        } while(--cnt);
        oed->oed_Succ = NULL;
        WRITEMEM32_LE(&oed->oed_Self, (IPTR)(&oed->oed_EPCaps) + hc->hc_PCIVirtualAdjust);
        memptr += sizeof(struct OhciED) * OHCI_ED_POOLSIZE;

        // build up TD pool
        otd = (struct OhciTD *) memptr;
        hc->hc_OhciTDPool = otd;
        cnt = OHCI_TD_POOLSIZE - 1;
        do {
            otd->otd_Succ = (otd + 1);
            WRITEMEM32_LE(&otd->otd_Self, (IPTR)(&otd->otd_Ctrl) + hc->hc_PCIVirtualAdjust);
            otd++;
        } while(--cnt);
        otd->otd_Succ = NULL;
        WRITEMEM32_LE(&otd->otd_Self, (IPTR)(&otd->otd_Ctrl) + hc->hc_PCIVirtualAdjust);
        memptr += sizeof(struct OhciTD) * OHCI_TD_POOLSIZE;

        // terminating ED
        hc->hc_OhciTermED = oed = ohciAllocED(hc);
        oed->oed_Succ = NULL;
        oed->oed_Pred = NULL;
        CONSTWRITEMEM32_LE(&oed->oed_EPCaps, OECF_SKIP);
        oed->oed_NextED = 0;

        // terminating TD
        hc->hc_OhciTermTD = otd = ohciAllocTD(hc);
        otd->otd_Succ = NULL;
        otd->otd_NextTD = 0;

        // dummy head & tail Ctrl ED
        hc->hc_OhciCtrlHeadED = predoed = ohciAllocED(hc);
        hc->hc_OhciCtrlTailED = oed = ohciAllocED(hc);
        CONSTWRITEMEM32_LE(&predoed->oed_EPCaps, OECF_SKIP);
        CONSTWRITEMEM32_LE(&oed->oed_EPCaps, OECF_SKIP);
        predoed->oed_Succ = oed;
        predoed->oed_Pred = NULL;
        predoed->oed_NextED = oed->oed_Self;
        oed->oed_Succ = NULL;
        oed->oed_Pred = predoed;
        oed->oed_NextED = 0;

        // dummy head & tail Bulk ED
        hc->hc_OhciBulkHeadED = predoed = ohciAllocED(hc);
        hc->hc_OhciBulkTailED = oed = ohciAllocED(hc);
        CONSTWRITEMEM32_LE(&predoed->oed_EPCaps, OECF_SKIP);
        CONSTWRITEMEM32_LE(&oed->oed_EPCaps, OECF_SKIP);
        predoed->oed_Succ = oed;
        predoed->oed_Pred = NULL;
        predoed->oed_NextED = oed->oed_Self;
        oed->oed_Succ = NULL;
        oed->oed_Pred = predoed;
        oed->oed_NextED = 0;
        // 1 ms INT QH
        hc->hc_OhciIntED[0] = oed = ohciAllocED(hc);
        oed->oed_Succ = hc->hc_OhciTermED;
        oed->oed_Pred = NULL; // who knows...
        CONSTWRITEMEM32_LE(&oed->oed_EPCaps, OECF_SKIP);
        oed->oed_NextED = hc->hc_OhciTermED->oed_Self;
        predoed = oed;
        // make 5 levels of QH interrupts
        for(cnt = 1; cnt < 5; cnt++) {
            hc->hc_OhciIntED[cnt] = oed = ohciAllocED(hc);
            oed->oed_Succ = predoed;
            oed->oed_Pred = NULL; // who knows...
            CONSTWRITEMEM32_LE(&oed->oed_EPCaps, OECF_SKIP);
            oed->oed_NextED = hc->hc_OhciTermED->oed_Self;
            predoed = oed;
        }

        ohciUpdateIntTree(hc);

        // fill in framelist with IntED entry points based on interval
        tabptr = hc->hc_OhciHCCA->oha_IntEDs;
        for(cnt = 0; cnt < 32; cnt++) {
            oed = hc->hc_OhciIntED[4];
            bitcnt = 0;
            do {
                if(cnt & (1UL<<bitcnt)) {
                    oed = hc->hc_OhciIntED[bitcnt];
                    break;
                }
            } while(++bitcnt < 5);
            *tabptr++ = oed->oed_Self;
        }

        // time to initialize hardware...
        OOP_GetAttr(hc->hc_PCIDeviceObject, aHidd_PCIDevice_Base0, (IPTR *) &hc->hc_RegBase);
        hc->hc_RegBase = (APTR) (((IPTR) hc->hc_RegBase) & (~0xf));
        KPRINTF(10, ("RegBase = 0x%08lx\n", hc->hc_RegBase));
        OOP_SetAttrs(hc->hc_PCIDeviceObject, (struct TagItem *) pciActivateMem); // enable memory

        hubdesca = READREG32_LE(hc->hc_RegBase, OHCI_HUBDESCA);
        hc->hc_NumPorts = (hubdesca & OHAM_NUMPORTS)>>OHAS_NUMPORTS;
        KPRINTF(20, ("Found OHCI Controller %08lx FuncNum = %ld, Rev %02lx, with %ld ports\n",
             hc->hc_PCIDeviceObject, hc->hc_FunctionNum,
             READREG32_LE(hc->hc_RegBase, OHCI_REVISION),
             hc->hc_NumPorts));

        KPRINTF(20, ("Powerswitching: %s %s\n",
             hubdesca & OHAF_NOPOWERSWITCH ? "Always on" : "Available",
             hubdesca & OHAF_INDIVIDUALPS ? "per port" : "global"));

        // disable BIOS legacy support
        control = READREG32_LE(hc->hc_RegBase, OHCI_CONTROL);
        if(control & OCLF_SMIINT) {
            KPRINTF(10, ("BIOS still has hands on OHCI, trying to get rid of it\n"));
            cmdstatus = READREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS);
            cmdstatus |= OCSF_OWNERCHANGEREQ;
            WRITEREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS, cmdstatus);
            timeout = 100;
            do {
                control = READREG32_LE(hc->hc_RegBase, OHCI_CONTROL);
                if(!(control & OCLF_SMIINT)) {
                    KPRINTF(10, ("BIOS gave up on OHCI. Pwned!\n"));
                    break;
                }
                uhwDelayMS(10, hu);
            } while(--timeout);
            if(!timeout) {
                KPRINTF(10, ("BIOS didn't release OHCI. Forcing and praying...\n"));
                control &= ~OCLF_SMIINT;
                WRITEREG32_LE(hc->hc_RegBase, OHCI_CONTROL, control);
            }
        }

        OOP_SetAttrs(hc->hc_PCIDeviceObject, (struct TagItem *) pciDeactivateBusmaster); // no busmaster yet

        KPRINTF(10, ("Resetting OHCI HC\n"));
        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS, OCSF_HCRESET);
        cnt = 100;
        do {
            if(!(READREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS) & OCSF_HCRESET)) {
                break;
            }
            uhwDelayMS(1, hu);
        } while(--cnt);

#ifdef DEBUG
        if(cnt == 0) {
            KPRINTF(20, ("Reset Timeout!\n"));
        } else {
            KPRINTF(20, ("Reset finished after %ld ticks\n", 100-cnt));
        }
#endif

        OOP_SetAttrs(hc->hc_PCIDeviceObject, (struct TagItem *) pciActivateBusmaster); // enable busmaster

        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_FRAMECOUNT, 0);
        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_PERIODICSTART, 10800); // 10% of 12000
        frameival = READREG32_LE(hc->hc_RegBase, OHCI_FRAMEINTERVAL);
        KPRINTF(10, ("FrameInterval=%08lx\n", frameival));
        frameival &= ~OIVM_BITSPERFRAME;
        frameival |= OHCI_DEF_BITSPERFRAME<<OIVS_BITSPERFRAME;
        frameival ^= OIVF_TOGGLE;
        WRITEREG32_LE(hc->hc_RegBase, OHCI_FRAMEINTERVAL, frameival);

        // make sure nothing is running
        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_PERIODIC_ED, 0);
        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_CTRL_HEAD_ED, AROS_LONG2LE(hc->hc_OhciCtrlHeadED->oed_Self));
        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_CTRL_ED, 0);
        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_BULK_HEAD_ED, AROS_LONG2LE(hc->hc_OhciBulkHeadED->oed_Self));
        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_BULK_ED, 0);
        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_DONEHEAD, 0);

        WRITEREG32_LE(hc->hc_RegBase, OHCI_HCCA, (IPTR)pciGetPhysical(hc, hc->hc_OhciHCCA));

        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_INTSTATUS, OISF_ALL_INTS);
        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_INTDIS, OISF_ALL_INTS);
        SYNC;

        // install reset handler
        hc->hc_ResetInt.is_Code = OhciResetHandler;
        hc->hc_ResetInt.is_Data = hc;
        AddResetCallback(&hc->hc_ResetInt);

        // add interrupt
        hc->hc_PCIIntHandler.h_Node.ln_Name = "OHCI PCI (pciusb.device)";
        hc->hc_PCIIntHandler.h_Node.ln_Pri = 5;
        hc->hc_PCIIntHandler.h_Code = ohciIntCode;
        hc->hc_PCIIntHandler.h_Data = hc;
        HIDD_IRQ_AddHandler(hd->hd_IRQHidd, &hc->hc_PCIIntHandler, hc->hc_PCIIntLine);

        hc->hc_PCIIntEnMask = OISF_DONEHEAD|OISF_RESUMEDTX|OISF_HOSTERROR|OISF_FRAMECOUNTOVER|OISF_HUBCHANGE;

        WRITEREG32_LE(hc->hc_RegBase, OHCI_INTEN, hc->hc_PCIIntEnMask|OISF_MASTERENABLE);

        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_CONTROL, OCLF_PERIODICENABLE|OCLF_CTRLENABLE|OCLF_BULKENABLE|OCLF_ISOENABLE|OCLF_USBRESET);
        SYNC;

        // make sure the ports are on with chipset quirk workaround
        hubdesca = READREG32_LE(hc->hc_RegBase, OHCI_HUBDESCA);
        WRITEREG32_LE(hc->hc_RegBase, OHCI_HUBDESCA, hubdesca|OHAF_NOOVERCURRENT);
        WRITEREG32_LE(hc->hc_RegBase, OHCI_HUBSTATUS, OHSF_POWERHUB);
        if((hubdesca & OHAF_NOPOWERSWITCH) || (!(hubdesca & OHAF_INDIVIDUALPS))) {
            KPRINTF(20, ("Individual power switching not available, turning on all ports!\n"));
            WRITEREG32_LE(hc->hc_RegBase, OHCI_HUBDESCB, 0);
        } else {
            KPRINTF(20, ("Enabling individual power switching\n"));
            WRITEREG32_LE(hc->hc_RegBase, OHCI_HUBDESCB, ((2<<hc->hc_NumPorts)-2)<<OHBS_PORTPOWERCTRL);
        }

        uhwDelayMS(50, hu);
        WRITEREG32_LE(hc->hc_RegBase, OHCI_HUBDESCA, hubdesca);

        CacheClearE(hc->hc_OhciHCCA,   sizeof(struct OhciHCCA),          CACRF_ClearD);
        CacheClearE(hc->hc_OhciEDPool, sizeof(struct OhciED) * OHCI_ED_POOLSIZE, CACRF_ClearD);
        CacheClearE(hc->hc_OhciTDPool, sizeof(struct OhciTD) * OHCI_TD_POOLSIZE, CACRF_ClearD);
            
        CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_CONTROL, OCLF_PERIODICENABLE|OCLF_CTRLENABLE|OCLF_BULKENABLE|OCLF_ISOENABLE|OCLF_USBOPER);
        SYNC;

        KPRINTF(20, ("ohciInit returns TRUE...\n"));
        return TRUE;
    }

    /*
        FIXME: What would the appropriate debug level be?
    */
    KPRINTF(1000, ("ohciInit returns FALSE...\n"));
    return FALSE;
}

void ohciFree(struct PCIController *hc, struct PCIUnit *hu) {

    hc = (struct PCIController *) hu->hu_Controllers.lh_Head;
    while(hc->hc_Node.ln_Succ)
    {
        switch(hc->hc_HCIType)
        {
            case HCITYPE_OHCI:
            {
                KPRINTF(20, ("Shutting down OHCI %08lx\n", hc));
                CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_INTDIS, OISF_ALL_INTS);
                CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_INTSTATUS, OISF_ALL_INTS);

                // disable all ports
                WRITEREG32_LE(hc->hc_RegBase, OHCI_HUBDESCB, 0);
                WRITEREG32_LE(hc->hc_RegBase, OHCI_HUBSTATUS, OHSF_UNPOWERHUB);

                uhwDelayMS(50, hu);
                KPRINTF(20, ("Stopping OHCI %08lx\n", hc));
                CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_CONTROL, 0);
                CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS, 0);
                SYNC;

                //KPRINTF(20, ("Reset done UHCI %08lx\n", hc));
                uhwDelayMS(10, hu);
                KPRINTF(20, ("Resetting OHCI %08lx\n", hc));
                CONSTWRITEREG32_LE(hc->hc_RegBase, OHCI_CMDSTATUS, OCSF_HCRESET);
                SYNC;
                uhwDelayMS(50, hu);

                KPRINTF(20, ("Shutting down OHCI done.\n"));
                break;
            }
        }

        hc = (struct PCIController *) hc->hc_Node.ln_Succ;
    }

}

