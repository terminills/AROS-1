/*
    (C) 2001 AROS - The Amiga Research OS
    $Id$

    Desc: 
    Lang: English
*/


#include <proto/exec.h>
#include <proto/dos.h>

#include "camd_intern.h"

#undef SysBase
#undef DosBase
#undef DOSBase


/*****************************************************
*****************************************************/

void Receiver_SetError(
	struct DriverData *driverdata,
	ULONG errorcode
){
	ULONG lock;
	struct MidiLink *midilink;
	struct MyMidiNode *mymidinode;

	lock=ObtainSharedSem(&driverdata->incluster->mutex);

		if(! (IsListEmpty(&driverdata->incluster->cluster.mcl_Receivers))){
			midilink=(struct MidiLink *)driverdata->incluster->cluster.mcl_Receivers.lh_Head;

			while(midilink->ml_Node.ln_Succ!=NULL){
				if(midilink->ml_Node.ln_Type!=NT_USER-MLTYPE_NTypes){
					mymidinode=(struct MyMidiNode *)midilink->ml_MidiNode;
					ObtainSemaphore(&mymidinode->receiversemaphore);
						mymidinode->error |= errorcode;
					ReleaseSemaphore(&mymidinode->receiversemaphore);
					midilink=(struct MidiLink *)midilink->ml_Node.ln_Succ;
				}
			}
		}

	ReleaseSharedSem(&driverdata->incluster->mutex,lock);
}


/*****************************************************
*****************************************************/

void Receiver_SysExSuperTreat(
	struct DriverData *driverdata,
	UBYTE data
){
	ULONG lock;
	struct MidiLink *midilink;
	struct MyMidiNode *mymidinode;

	lock=ObtainSharedSem(&driverdata->incluster->mutex);

		if(! (IsListEmpty(&driverdata->incluster->cluster.mcl_Receivers))){
			midilink=(struct MidiLink *)driverdata->incluster->cluster.mcl_Receivers.lh_Head;

			while(midilink->ml_Node.ln_Succ!=NULL){
				if(midilink->ml_Node.ln_Type!=NT_USER-MLTYPE_NTypes){
					mymidinode=(struct MyMidiNode *)midilink->ml_MidiNode;
					ObtainSemaphore(&mymidinode->receiversemaphore);
						PutSysEx2Link(midilink,data);
					ReleaseSemaphore(&mymidinode->receiversemaphore);
					midilink=(struct MidiLink *)midilink->ml_Node.ln_Succ;
				}
			}
		}

	ReleaseSharedSem(&driverdata->incluster->mutex,lock);

}

void Receiver_SuperTreat2(
	struct DriverData *driverdata,
	struct MyMidiMessage2 *msg2
){
	ULONG lock;
	struct MidiLink *midilink;
	struct MyMidiNode *mymidinode;
	ULONG msg;

	lock=ObtainSharedSem(&driverdata->incluster->mutex);

		if(! (IsListEmpty(&driverdata->incluster->cluster.mcl_Receivers))){

			midilink=(struct MidiLink *)driverdata->incluster->cluster.mcl_Receivers.lh_Head;

			while(midilink->ml_Node.ln_Succ!=NULL){
				if(midilink->ml_Node.ln_Type==NT_USER-MLTYPE_NTypes){	// Only happens if called from ParseMidi
					if(driverdata->lastsysex==NULL){		// If a realtime-message are inside of a sysexmessage.
						msg=(msg2->status<<24)|(msg2->data1<<16)|(msg2->data2<<8);
						while(Midi2Driver((struct DriverData *)midilink,msg,10000)==FALSE){
							Delay(1);
						}
					}
				}else{
					mymidinode=(struct MyMidiNode *)midilink->ml_MidiNode;
					ObtainSemaphore(&mymidinode->receiversemaphore);
						PutMidi2Link(
							midilink,
							msg2,
							*mymidinode->midinode.mi_TimeStamp
						);
					ReleaseSemaphore(&mymidinode->receiversemaphore);
				}
				midilink=(struct MidiLink *)midilink->ml_Node.ln_Succ;
			}
		}

	ReleaseSharedSem(&driverdata->incluster->mutex,lock);

}

#define Receiver_SuperTreat(a) Receiver_SuperTreat2(driverdata,&driverdata->msg2);



/******************************************
	Status
******************************************/

void Receiver_NewSysEx(
	struct DriverData *driverdata
);
void Receiver_NewSysCom(
	struct DriverData *driverdata,
	UBYTE status
);
void Receiver_General3_first(
	struct DriverData *driverdata,
	UBYTE data
);
void Receiver_General2_first(
	struct DriverData *driverdata,
	UBYTE data
);

void Receiver_NewStatus(
	struct DriverData *driverdata,
	UBYTE status
){

	driverdata->msg2.status=status;

	switch(status & 0xf0){
		case 0x80:
		case 0x90:
		case 0xa0:
		case 0xb0:
		case 0xe0:
			driverdata->msg2.len=3;
			driverdata->Input_Treat=Receiver_General3_first;
			break;
		case 0xc0:
		case 0xd0:
			driverdata->msg2.len=2;
			driverdata->Input_Treat=Receiver_General2_first;
			break;
		case 0xf0:
			if(status==0xf0){
				Receiver_NewSysEx(driverdata);
			}else{
				Receiver_NewSysCom(driverdata,status);
			}
			break;
	}
}

void Receiver_ErrorAndNewStatus(
	struct DriverData *driverdata,
	UBYTE status
){
	Receiver_SetError(driverdata,CMEF_MsgErr);
	Receiver_NewStatus(driverdata,status);
}

void Receiver_NewStatus_first(
	struct DriverData *driverdata,
	UBYTE status
){
	if(status<0x80){
		Receiver_SetError(driverdata,CMEF_MsgErr);
	}else{
		Receiver_NewStatus(driverdata,status);
	}
}

/******************************************
	SysEx
******************************************/

void Receiver_SysEx(
	struct DriverData *driverdata,
	UBYTE data
){
	if(data>=0x80){
		if(data==0xf7){
			Receiver_SysExSuperTreat(driverdata,0xf7);
			driverdata->Input_Treat=Receiver_NewStatus_first;
		}else{
			Receiver_SysExSuperTreat(driverdata,0xff);
			Receiver_ErrorAndNewStatus(driverdata,data);
		}
	}else{
		Receiver_SysExSuperTreat(driverdata,data);
	}
	return;
}

void Receiver_NewSysEx(
	struct DriverData *driverdata
){
	Receiver_SysExSuperTreat(driverdata,0xf0);
	driverdata->Input_Treat=Receiver_SysEx;
	return;
}



/******************************************
	System Common
******************************************/

void Receiver_SysCom3_2(
	struct DriverData *driverdata,
	UBYTE data
){
	if(data>=0x80){
		Receiver_ErrorAndNewStatus(driverdata,data);
	}else{
		driverdata->msg2.data2=data;
		Receiver_SuperTreat(driverdata);
		driverdata->Input_Treat=Receiver_NewStatus_first;
	}
}

void Receiver_SysCom3_1(
	struct DriverData *driverdata,
	UBYTE data
){
	if(data>=0x80){
		Receiver_ErrorAndNewStatus(driverdata,data);
	}else{
		driverdata->msg2.data1=data;
		driverdata->Input_Treat=Receiver_SysCom3_2;
	}
}

void Receiver_SysCom2(
	struct DriverData *driverdata,
	UBYTE data
){
	if(data>=0x80){
		Receiver_ErrorAndNewStatus(driverdata,data);
	}else{
		driverdata->msg2.data1=data;
		Receiver_SuperTreat(driverdata);
		driverdata->Input_Treat=Receiver_NewStatus_first;
	}
}


void Receiver_NewSysCom(
	struct DriverData *driverdata,
	UBYTE status
){
	switch(status){
		case 0xf2:
			driverdata->msg2.len=3;
			driverdata->Input_Treat=Receiver_SysCom3_1;
			break;
		case 0xf1:
		case 0xf3:
			driverdata->msg2.len=2;
			driverdata->Input_Treat=Receiver_SysCom2;
			break;
		case 0xf6:
			driverdata->msg2.len=1;
			Receiver_SuperTreat(driverdata);
			driverdata->Input_Treat=Receiver_NewStatus_first;
			break;
		case 0xf7:
			Receiver_SetError(driverdata,CMEF_MsgErr);
			driverdata->Input_Treat=Receiver_NewStatus_first;
			break;
		default:
			// Undefined SysCom. Topic: should the error not be set?
			Receiver_SetError(driverdata,CMEF_MsgErr);
			driverdata->Input_Treat=Receiver_NewStatus_first;
			break;
	}
	return;
}



/******************************************
	Channel messages, length 3.
******************************************/

void Receiver_General3_1(struct DriverData *driverdata,UBYTE data);

void Receiver_General3_2(
	struct DriverData *driverdata,
	UBYTE data
){
	if(data>=0x80){
		Receiver_ErrorAndNewStatus(driverdata,data);
	}else{
		driverdata->msg2.data2=data;
		Receiver_SuperTreat(driverdata);
		driverdata->Input_Treat=Receiver_General3_1;
	}
}

void Receiver_General3_1(
	struct DriverData *driverdata,
	UBYTE data
){
	if(data>=0x80){
		Receiver_NewStatus(driverdata,data);
	}else{
		driverdata->msg2.data1=data;
		driverdata->Input_Treat=Receiver_General3_2;
	}
}

void Receiver_General3_first(
	struct DriverData *driverdata,
	UBYTE data
){
	if(data>=0x80){
		Receiver_ErrorAndNewStatus(driverdata,data);
	}else{
		driverdata->msg2.data1=data;
		driverdata->Input_Treat=Receiver_General3_2;
	}
}


/******************************************
	Channel messages, length 2.
******************************************/

void Receiver_General2(
	struct DriverData *driverdata,
	UBYTE data
){
	if(data>=0x80){
		Receiver_NewStatus(driverdata,data);
	}else{
		driverdata->msg2.data1=data;
		Receiver_SuperTreat(driverdata);
	}
	return;
}

void Receiver_General2_first(
	struct DriverData *driverdata,
	UBYTE data
){
	if(data>=0x80){
		Receiver_ErrorAndNewStatus(driverdata,data);
	}else{
		driverdata->msg2.data1=data;
		Receiver_SuperTreat(driverdata);
		driverdata->Input_Treat=Receiver_General2;
	}
	return;
}

/******************************************
	Realtime
******************************************/

void Receiver_RealTime(
	struct DriverData *driverdata,
	UBYTE status
){
	struct MyMidiMessage2 msg2;
	msg2.status=status;
	msg2.len=1;
	Receiver_SuperTreat2(driverdata,&msg2);
	return;
}

/******************************************
	Init, only used until first non-data
	non-realtime non-sysex-end message is received.
******************************************/

void Receiver_init(
	struct DriverData *driverdata,
	UBYTE data
){
	if(data<0x80 || data==0xf7) return;

	Receiver_NewStatus(driverdata,data);
}



/******************************************
	First function to be called from the
	receiver process.
******************************************/

void Receiver_first(
	struct DriverData *driverdata
){
	UWORD input;

	input=*driverdata->re_read;
	driverdata->re_read++;
	if(driverdata->re_read==driverdata->re_end){
		driverdata->re_read=driverdata->re_start;
	}

	if(input&0x100){
		Receiver_SetError(driverdata,CMEF_RecvOverflow);
	}

	input&=0xff;

	if(input>=0xf8){
		Receiver_RealTime(driverdata,input);
	}else{
		(*driverdata->Input_Treat)(driverdata,input);
	}

	driverdata->unpicked--;
}



/******************************************
	Code called from driver.
******************************************/

SAVEDS void ASM Receiver(
	REG(d0) UWORD input,
	REG(a2) struct DriverData *driverdata
){
	*driverdata->re_write=input;
	driverdata->re_write++;
	if(driverdata->re_write==driverdata->re_end){
		driverdata->re_write=driverdata->re_start;
	}
	driverdata->unpicked++;
	Signal(&driverdata->ReceiverProc->pr_Task,1L<<driverdata->ReceiverSig);

}



