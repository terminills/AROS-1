/*
    Copyright � 2014, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/

#include <aros/macros.h>

#include <proto/exec.h>
#include <proto/arossupport.h>

#include <devices/usb.h>
#include <devices/usb_hub.h>
#include <devices/usbhardware.h>

#include <stdio.h>

#define DEBUG 1
#include <aros/debug.h>

#include LC_LIBDEFS_FILE

WORD cmdQueryDevice(struct IOUsbHWReq *ioreq) {
    struct TagItem *taglist = (struct TagItem *) ioreq->iouh_Data;
    struct TagItem *tag;
    ULONG count = 0;

    while((tag = LibNextTagItem(&taglist)) != NULL) {
        switch (tag->ti_Tag) {
            case UHA_Manufacturer:
                *((STRPTR *) tag->ti_Data) = "The AROS Development Team";
                count++;
                break;
            case UHA_Version:
                *((ULONG *) tag->ti_Data) = VERSION_NUMBER;
                count++;
                break;
            case UHA_Revision:
                *((ULONG *) tag->ti_Data) = REVISION_NUMBER;
                count++;
                break;
            case UHA_Copyright:
                *((STRPTR *) tag->ti_Data) ="�2014 The AROS Development Team";
                count++;
                break;
            case UHA_ProductName:
                *((STRPTR *) tag->ti_Data) = "VXHCI";
                count++;
                break;
            case UHA_Capabilities:
                *((ULONG *) tag->ti_Data) = (UHCF_USB20|UHCF_USB30);
                count++;
                break;
            default:
                break;
        }
    }

    ioreq->iouh_Actual = count;
    return RC_OK;
}

WORD cmdUsbReset(struct IOUsbHWReq *ioreq) {
    bug("[VXHCI] cmdUsbReset: Entering function\n");

    struct VXHCIUnit *unit = (struct VXHCIUnit *) ioreq->iouh_Req.io_Unit;

    /* We should do a proper reset sequence with a real driver */
    unit->state = UHSF_RESET;
    unit->roothub.addr = 0;
    unit->state = UHSF_RESUMING;
    unit->state = UHSF_OPERATIONAL;
    bug("[VXHCI] cmdUsbReset: Done\n\n");
    return RC_OK;
}

WORD cmdControlXFer(struct IOUsbHWReq *ioreq) {
    bug("[VXHCI] cmdControlXFer: Entering function\n");

    struct VXHCIUnit *unit = (struct VXHCIUnit *) ioreq->iouh_Req.io_Unit;

    bug("[VXHCI] cmdControlXFer: ioreq->iouh_DevAddr %lx\n", ioreq->iouh_DevAddr);
    bug("[VXHCI] cmdControlXFer: unit->roothub.addr %lx\n", unit->roothub.addr);

    /*
        Check the status of the controller
        We might encounter these states:
        UHSB_OPERATIONAL USB can be used for transfers
        UHSB_RESUMING    USB is currently resuming
        UHSB_SUSPENDED   USB is in suspended state
        UHSB_RESET       USB is just inside a reset phase
    */
    if(unit->state == UHSF_OPERATIONAL) {
        bug("[VXHCI] cmdControlXFer: Unit state: UHSF_OPERATIONAL\n");
    } else {
        bug("[VXHCI] cmdControlXFer: Unit state: UHSF_SUSPENDED\n");
        return UHIOERR_USBOFFLINE;
    }

    if(ioreq->iouh_DevAddr == unit->roothub.addr) {
        return(cmdControlXFerRootHub(ioreq));
    }

    return RC_DONTREPLY;
}

WORD cmdControlXFerRootHub(struct IOUsbHWReq *ioreq) {
    bug("[VXHCI] cmdControlXFerRootHub: Entering function\n");

    //UWORD rt = ioreq->iouh_SetupData.bmRequestType;
    //UWORD req = ioreq->iouh_SetupData.bRequest;
    //UWORD idx = AROS_WORD2LE(ioreq->iouh_SetupData.wIndex);
    //UWORD val = AROS_WORD2LE(ioreq->iouh_SetupData.wValue);
    //UWORD len = AROS_WORD2LE(ioreq->iouh_SetupData.wLength);

    UWORD bmRequestType      = (ioreq->iouh_SetupData.bmRequestType) & (URTF_STANDARD | URTF_CLASS | URTF_VENDOR);
    UWORD bmRequestDirection = (ioreq->iouh_SetupData.bmRequestType) & (URTF_IN | URTF_OUT);
    UWORD bmRequestRecipient = (ioreq->iouh_SetupData.bmRequestType) & (URTF_DEVICE | URTF_INTERFACE | URTF_ENDPOINT | URTF_OTHER);

    UWORD bRequest           = (ioreq->iouh_SetupData.bRequest);
    UWORD wIndex             = AROS_WORD2LE(ioreq->iouh_SetupData.wIndex);
    UWORD wValue             = AROS_WORD2LE(ioreq->iouh_SetupData.wValue);
    UWORD wLength            = AROS_WORD2LE(ioreq->iouh_SetupData.wLength);

    struct VXHCIUnit *unit = (struct VXHCIUnit *) ioreq->iouh_Req.io_Unit;

    bug("[VXHCI] cmdControlXFerRootHub: Endpoint number is %ld \n", ioreq->iouh_Endpoint);

    bug("[VXHCI] cmdControlXFerRootHub: bmRequestDirection ");
    switch (bmRequestDirection) {
        case URTF_IN:
            bug("URTF_IN\n");
            break;
        case URTF_OUT:
            bug("URTF_OUT\n");
            break;
    }

    bug("[VXHCI] cmdControlXFerRootHub: bmRequestType ");
    switch(bmRequestType) {
        case URTF_STANDARD:
            bug("URTF_STANDARD\n");
            break;
        case URTF_CLASS:
            bug("URTF_CLASS\n");
            break;
        case URTF_VENDOR:
            bug("URTF_VENDOR\n");
            break;
    }

    bug("[VXHCI] cmdControlXFerRootHub: bmRequestRecipient ");
    switch (bmRequestRecipient) {
        case URTF_DEVICE:
            bug("URTF_DEVICE\n");
            break;
        case URTF_INTERFACE:
            bug("URTF_INTERFACE\n");
            break;
        case URTF_ENDPOINT:
            bug("URTF_ENDPOINT\n");
            break;
        case URTF_OTHER:
            bug("URTF_OTHER\n");
            break;
    }

    bug("[VXHCI] cmdControlXFerRootHub: bRequest ");
    switch(bRequest) {
        case USR_GET_STATUS:
            bug("USR_GET_STATUS\n");
            break;
        case USR_CLEAR_FEATURE:
            bug("USR_CLEAR_FEATURE\n");
            break;
        case USR_SET_FEATURE:
            bug("USR_SET_FEATURE\n");
            break;
        case USR_SET_ADDRESS:
            bug("USR_SET_ADDRESS\n");
            break;
        case USR_GET_DESCRIPTOR:
            bug("USR_GET_DESCRIPTOR\n");
            break;
        case USR_SET_DESCRIPTOR:
            bug("USR_SET_DESCRIPTOR\n");
            break;
        case USR_GET_CONFIGURATION:
            bug("USR_GET_CONFIGURATION\n");
            break;
        case USR_SET_CONFIGURATION:
            bug("USR_SET_CONFIGURATION\n");
            break;
        case USR_GET_INTERFACE:
            bug("USR_GET_INTERFACE\n");
            break;
        case USR_SET_INTERFACE:
            bug("USR_SET_INTERFACE\n");
            break;
        case USR_SYNCH_FRAME:
            bug("USR_SYNCH_FRAME\n");
            break;
    }

    bug("[VXHCI] cmdControlXFerRootHub: wIndex %x\n", wIndex);
    bug("[VXHCI] cmdControlXFerRootHub: wValue %x\n", wValue);
    bug("[VXHCI] cmdControlXFerRootHub: wLength %d\n", wLength);

    /* Endpoint 0 is used for control transfers only and can not be assigned to any other function. */
    if(ioreq->iouh_Endpoint != 0) {
        return UHIOERR_BADPARAMS;
    }

    /* Check the request */
    if(bmRequestDirection) {
        bug("[VXHCI] cmdControlXFerRootHub: Request direction is device to host\n");

        switch(bmRequestType) {
            case URTF_STANDARD:
                bug("[VXHCI] cmdControlXFerRootHub: URTF_STANDARD\n");

                switch(bmRequestRecipient) {
                    case URTF_DEVICE:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_DEVICE\n");

                        switch(bRequest) {
                            case USR_GET_DESCRIPTOR:
                                bug("[VXHCI] cmdControlXFerRootHub: USR_GET_DESCRIPTOR\n");

                                switch( (wValue>>8) ) {
                                    case UDT_DEVICE:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_DEVICE\n");
                                        bug("[VXHCI] cmdControlXFerRootHub: GetDeviceDescriptor (%ld)\n", wLength);

                                        /*
                                            Poseidon first does a dummy psdPipeSetup(URTF_IN|URTF_STANDARD|URTF_DEVICE, USR_GET_DESCRIPTOR, UDT_DEVICE)
                                            with 8 byte transfer size. It will then set the address with psdPipeSetup(URTF_STANDARD|URTF_DEVICE, USR_SET_ADDRESS)
                                            After that Poseidon does again psdPipeSetup(URTF_IN|URTF_STANDARD|URTF_DEVICE, USR_GET_DESCRIPTOR, UDT_DEVICE) with
                                            8 byte transfer size to get the bMaxPacketSize0 for transfer sizes.
                                            Only after that will it read the whole descriptor.
                                        */
                                        ioreq->iouh_Actual = (wLength > sizeof(struct UsbStdDevDesc)) ? sizeof(struct UsbStdDevDesc) : wLength;
                                        CopyMem((APTR) &unit->roothub.devdesc, ioreq->iouh_Data, ioreq->iouh_Actual);

                                        bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                        return(0);
                                        break;

                                    case UDT_CONFIGURATION:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_CONFIGURATION\n");
                                        bug("[VXHCI] cmdControlXFerRootHub: GetConfigDescriptor (%ld)\n", wLength);

                                        ioreq->iouh_Actual = (wLength > sizeof(struct RHConfig)) ? sizeof(struct RHConfig) : wLength;
                                        CopyMem((APTR) &unit->roothub.config, ioreq->iouh_Data, ioreq->iouh_Actual);

                                        //bug("sizeof(struct RHConfig) = %ld (should be 25)\n", sizeof(struct RHConfig));
                                        bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                        return(0);

                                        break;

                                    case UDT_STRING:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_STRING id %d\n", (wValue & 0xff));

                                        if(wLength > 1) {
                                            switch( (wValue & 0xff) ) {
                                                case 0:
                                                    bug("[VXHCI] cmdControlXFerRootHub: GetStringDescriptor (%ld)\n", wLength);

                                                    struct UsbStdStrDesc *strdesc = (struct UsbStdStrDesc *) ioreq->iouh_Data;

                                                    /* This is our root hub string descriptor */
                                                    if(wLength > 3) {
                                                        strdesc->bString[1] = AROS_WORD2LE(0x0409); // English (Yankee)
                                                        ioreq->iouh_Actual = sizeof(struct UsbStdStrDesc);
                                                        bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                                        return(0);
                                                    } else {
                                                        strdesc->bLength         = sizeof(struct UsbStdStrDesc);
                                                        strdesc->bDescriptorType = UDT_STRING;
                                                        ioreq->iouh_Actual = wLength;
                                                        bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                                        return(0);
                                                    }

                                                    break;

                                                case 1:
                                                    return cmdGetString(ioreq, "The AROS Development Team.");
                                                    break;

                                                case 2: {
                                                    char roothubname[100];
                                                    sprintf(roothubname, "VXHCI USB%d.0 Root Hub", (unit->roothub.devdesc.bcdUSB == 0x200) ? 2 : 3);
                                                    return cmdGetString(ioreq, roothubname);
                                                    break;
                                                    }

                                                case 3:
                                                    return cmdGetString(ioreq, "Standard Config");
                                                    break;

                                                case 4:
                                                    return cmdGetString(ioreq, "Hub interface");
                                                    break;

                                                default:
                                                    break;
                                            }
                                        }

                                        break;

                                    case UDT_INTERFACE:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_INTERFACE\n");
                                        break;

                                    case UDT_ENDPOINT:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_ENDPOINT\n");
                                        break;

                                    case UDT_DEVICE_QUALIFIER:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_DEVICE_QUALIFIER\n");
                                        break;

                                    case UDT_OTHERSPEED_QUALIFIER:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_OTHERSPEED_QUALIFIER\n");
                                        break;

                                    case UDT_INTERFACE_POWER:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_INTERFACE_POWER\n");
                                        break;

                                    case UDT_OTG:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_OTG\n");
                                        break;

                                    case UDT_DEBUG:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_DEBUG\n");
                                        break;

                                    case UDT_INTERFACE_ASSOCIATION:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_INTERFACE_ASSOCIATION\n");
                                        break;

                                    case UDT_SECURITY:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_SECURITY\n");
                                        break;

                                    case UDT_ENCRYPTION_TYPE:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_ENCRYPTION_TYPE\n");
                                        break;

                                    case UDT_BOS:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_BOS\n");
                                        break;

                                    case UDT_DEVICE_CAPABILITY:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_DEVICE_CAPABILITY\n");
                                        break;

                                    case UDT_WIRELESS_EP_COMP:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_WIRELESS_EP_COMP\n");
                                        break;

                                    default:
                                        bug("[VXHCI] cmdControlXFerRootHub: switch( (wValue>>8) ) %ld\n", (wValue>>8));
                                        break;

                                } /* switch( (wValue>>8) ) */
                                break; /* case USR_GET_DESCRIPTOR */

                            case USR_GET_STATUS:
                                bug("[VXHCI] cmdControlXFerRootHub: USR_GET_STATUS\n");
                                ((UWORD *) ioreq->iouh_Data)[0] = AROS_WORD2LE(U_GSF_SELF_POWERED);
                                ioreq->iouh_Actual = wLength;
                                bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                return(0);
                                break;

                        } /* switch(bRequest) */
                        break; /* case URTF_DEVICE: */

                    case URTF_INTERFACE:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_INTERFACE\n");
                        break;

                    case URTF_ENDPOINT:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_ENDPOINT\n");
                        break;

                    case URTF_OTHER:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_OTHER\n");
                        break;

                    default:
                        bug("[VXHCI] cmdControlXFerRootHub: %ld\n", bRequest);
                        break;

                } /* switch(bmRequestRecipient) */
                break;

            case URTF_CLASS:
                bug("[VXHCI] cmdControlXFerRootHub: URTF_CLASS\n");

                switch(bmRequestRecipient) {
                    case URTF_DEVICE:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_DEVICE\n");

                        switch(bRequest) {
                            case USR_GET_STATUS:
                                bug("[VXHCI] cmdControlXFerRootHub: USR_GET_STATUS\n");
                                UWORD *mptr = ioreq->iouh_Data;
                                if(wLength < sizeof(struct UsbHubStatus)) {
                                    return(UHIOERR_STALL);
                                }
                                *mptr++ = 0;
                                *mptr++ = 0;
                                ioreq->iouh_Actual = 4;
                                bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                return(0);
                                break;


                            case USR_GET_DESCRIPTOR:
                                bug("[VXHCI] cmdControlXFerRootHub: USR_GET_DESCRIPTOR\n");

                                switch( (wValue>>8) ) {
                                    case UDT_HUB:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_HUB\n");
                                        bug("[VXHCI] cmdControlXFerRootHub: GetRootHubDescriptor USB2.0 (%ld)\n", wLength);

                                        ioreq->iouh_Actual = (wLength > sizeof(struct UsbHubDesc)) ? sizeof(struct UsbHubDesc) : wLength;
                                        CopyMem((APTR) &unit->roothub.hubdesc.usb20, ioreq->iouh_Data, ioreq->iouh_Actual);

                                        bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                        return(0);
                                        break;

                                    case UDT_SSHUB:
                                        bug("[VXHCI] cmdControlXFerRootHub: UDT_SSHUB\n");
                                        bug("[VXHCI] cmdControlXFerRootHub: GetRootHubDescriptor USB3.0 (%ld)\n", wLength);

                                        ioreq->iouh_Actual = (wLength > sizeof(struct UsbSSHubDesc)) ? sizeof(struct UsbSSHubDesc) : wLength;
                                        CopyMem((APTR) &unit->roothub.hubdesc.usb30, ioreq->iouh_Data, ioreq->iouh_Actual);

                                        bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                        return(0);
                                        break;
                                }

                                bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                return(0);
                                break;
                        }
                        break;

                } /* case URTF_CLASS */
                break;

            case URTF_VENDOR:
                bug("[VXHCI] cmdControlXFerRootHub: URTF_VENDOR\n");
                break;
        } /* switch(bmRequestType) */

    } else { /* if(bmRequestDirection) */
        bug("[VXHCI] cmdControlXFerRootHub: Request direction is host to device\n");

        switch(bmRequestType) {
            case URTF_STANDARD:
                bug("[VXHCI] cmdControlXFerRootHub: URTF_STANDARD\n");

                switch(bmRequestRecipient) {
                    case URTF_DEVICE:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_DEVICE\n");

                        switch(bRequest) {
                            case USR_SET_ADDRESS:
                                bug("[VXHCI] cmdControlXFerRootHub: USR_SET_ADDRESS\n");
                                unit->roothub.addr = wValue;
                                ioreq->iouh_Actual = wLength;
                                bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                return(0);
                                break;

                            case USR_SET_CONFIGURATION:
                                /* We do not have alternative configuration */
                                bug("[VXHCI] cmdControlXFerRootHub: USR_SET_CONFIGURATION\n");
                                ioreq->iouh_Actual = wLength;
                                bug("[VXHCI] cmdControlXFerRootHub: Done\n\n");
                                return(0);
                                break;

                        } /* switch(bRequest) */
                        break;

                    case URTF_INTERFACE:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_INTERFACE\n");
                        break;

                    case URTF_ENDPOINT:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_ENDPOINT\n");
                        break;

                    case URTF_OTHER:
                        bug("[VXHCI] cmdControlXFerRootHub: URTF_OTHER\n");
                        break;

                } /* switch(bmRequestRecipient) */
                break;

            case URTF_CLASS:
                bug("[VXHCI] cmdControlXFerRootHub: URTF_CLASS\n");
                break;

            case URTF_VENDOR:
                bug("[VXHCI] cmdControlXFerRootHub: URTF_VENDOR\n");
                break;

        } /* switch(bmRequestType) */

    } /* if(bmRequestDirection) */

    return UHIOERR_BADPARAMS;
}

WORD cmdIntXFer(struct IOUsbHWReq *ioreq) {
    bug("[VXHCI] cmdIntXFer: Entering function\n");

    struct VXHCIUnit *unit = (struct VXHCIUnit *) ioreq->iouh_Req.io_Unit;

    bug("[VXHCI] cmdIntXFer: ioreq->iouh_DevAddr %lx\n", ioreq->iouh_DevAddr);
    bug("[VXHCI] cmdIntXFer: unit->roothub.addr %lx\n", unit->roothub.addr);

    /*
        Check the status of the controller
        We might encounter these states:
        UHSB_OPERATIONAL USB can be used for transfers
        UHSB_RESUMING    USB is currently resuming
        UHSB_SUSPENDED   USB is in suspended state
        UHSB_RESET       USB is just inside a reset phase
    */
    if(unit->state == UHSF_OPERATIONAL) {
        bug("[VXHCI] cmdControlXFer: Unit state: UHSF_OPERATIONAL\n");
    } else {
        bug("[VXHCI] cmdControlXFer: Unit state: UHSF_SUSPENDED\n");
        return UHIOERR_USBOFFLINE;
    }

    if(ioreq->iouh_DevAddr == unit->roothub.addr) {
        return(cmdIntXFerRootHub(ioreq));
    }

    return RC_DONTREPLY;
}

WORD cmdIntXFerRootHub(struct IOUsbHWReq *ioreq) {
    bug("[VXHCI] cmdIntXFerRootHub: Entering function\n");

    return RC_DONTREPLY;
}

WORD cmdGetString(struct IOUsbHWReq *ioreq, char *cstring) {
    bug("[VXHCI] cmdGetString: Entering function\n");

    UWORD wLength = AROS_WORD2LE(ioreq->iouh_SetupData.wLength);

    struct UsbStdStrDesc *strdesc = (struct UsbStdStrDesc *) ioreq->iouh_Data;
    strdesc->bDescriptorType = UDT_STRING;
    strdesc->bLength = (strlen(cstring)*sizeof(strdesc->bString))+sizeof(strdesc->bLength) + sizeof(strdesc->bDescriptorType);

    if(wLength > 2) {
        ioreq->iouh_Actual = 2;
        while(ioreq->iouh_Actual<wLength) {
            strdesc->bString[(ioreq->iouh_Actual-2)/sizeof(strdesc->bString)] = AROS_WORD2LE(*cstring);
            ioreq->iouh_Actual += sizeof(strdesc->bString);
            cstring++;
            if(*cstring == 0) {
                bug("[VXHCI] cmdGetString: Done\n\n");
                return(0);
            }
        }

    } else {
        ioreq->iouh_Actual = wLength;
        bug("[VXHCI] cmdGetString: Done\n\n");
        return(0);
    }

    return UHIOERR_BADPARAMS;
}
