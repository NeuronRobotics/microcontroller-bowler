/**
 * @file Namespace_bcs_io.c
 *
 * Created on: Mar 16, 2013
 * @author hephaestus
 */


#include "Bowler/Bowler.h"
#include "Namespace/Namespace_bcs_io.h"

const unsigned char ioNSName[] = "bcs.io.*;0.3;;";
RunEveryData ppm={0,200};



BOOL bcsIoAsyncEventCallback(BOOL (*pidAsyncCallbackPtr)(BowlerPacket *Packet)){
	runAsyncIO();
	if (RunEvery(&ppm)>0)
		RunPPMCheck();

    return FALSE;
}

BOOL bcsIoProcessor_g(BowlerPacket * Packet){

	switch (Packet->use.head.RPC){

	case GCHV:
		if(!GetChannelValue(Packet)){
			//ERR(Packet,zone,0);
		}
		break;
	case GACV:
		populateGACV(Packet);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

BOOL bcsIoProcessor_p(BowlerPacket * Packet){
	int zone=2;
	switch (Packet->use.head.RPC){
	case SCHV:
		if(SetChannelValue(Packet))
			READY(Packet,zone,2);
		else
			ERR(Packet,zone,2);
		break;
	case SACV:
		SetAllChannelValue(Packet);
		break;
	case ASYN:
		Packet->use.head.Method=BOWLER_POST;
		setAsync(Packet->use.data[0],Packet->use.data[1]);
		Packet->use.head.DataLegnth=4;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL bcsIoProcessor_c(BowlerPacket * Packet){
	int zone=2;
	switch (Packet->use.head.RPC){
	case CCHN:
		SendPacketToCoProc(Packet);
		break;
	case SCHV:
		getBcsIoDataTable()[Packet->use.data[0]].PIN.ServoPos=Packet->use.data[1];
		SendPacketToCoProc(Packet);
		break;
	case ASYN:
		setAsync(Packet->use.data[0],TRUE);
		configAdvancedAsync(Packet);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

// GET structures
static RPC_LIST bcsIo_gchm_g={	BOWLER_GET,// Method
                                "gchm",//RPC as string
                                &GetChannelModeFromPacket,//function pointer to a packet parsing function
                                NULL //Termination
};

static RPC_LIST bcsIo_gacm_g={	BOWLER_GET,// Method
                                "gacm",//RPC as string
                                &GetAllChannelModeFromPacket,//function pointer to a packet parsing function
                                NULL //Termination
};

static RPC_LIST bcsIo_gchv_g={	BOWLER_GET,// Method
                                "gchv",//RPC as string
                                &bcsIoProcessor_g,//function pointer to a packet parsing function
                                NULL //Termination
};
static RPC_LIST bcsIo_gacv_g={	BOWLER_GET,// Method
                                "gacv",//RPC as string
                                &bcsIoProcessor_g,//function pointer to a packet parsing function
                                NULL //Termination
};
static RPC_LIST bcsIo_asyn_g={	BOWLER_GET,// Method
                                "asyn",//RPC as string
                                &GetAsyncFromPacket,//function pointer to a packet parsing function
                                NULL //Termination
};
static RPC_LIST bcsIo_gchc_g={	BOWLER_GET,// Method
                                "gchc",//RPC as string
                                &GetIOChannelCountFromPacket,//function pointer to a packet parsing function
                                NULL //Termination
};
static RPC_LIST bcsIo_gcml_g={	BOWLER_GET,// Method
                                "gcml",//RPC as string
                                &getFunctionList,//function pointer to a packet parsing function
                                NULL //Termination
};
//POST
static RPC_LIST bcsIo_schv_p={	BOWLER_POST,// Method
                                "schv",//RPC as string
                                &bcsIoProcessor_p,//function pointer to a packet parsing function
                                NULL //Termination
};
static RPC_LIST bcsIo_sacv_p={	BOWLER_POST,// Method
                                "sacv",//RPC as string
                                &bcsIoProcessor_p,//function pointer to a packet parsing function
                                NULL //Termination
};
static RPC_LIST bcsIo_asyn_p={	BOWLER_POST,// Method
                                "asyn",//RPC as string
                                &bcsIoProcessor_p,//function pointer to a packet parsing function
                                NULL //Termination
};
//CRIT
static RPC_LIST bcsIo_cchn_c={	BOWLER_CRIT,// Method
                                "cchn",//RPC as string
                                &bcsIoProcessor_c,//function pointer to a packet parsing function
                                NULL //Termination
};
static RPC_LIST bcsIo_schv_c={	BOWLER_CRIT,// Method
                                "schv",//RPC as string
                                &bcsIoProcessor_c,//function pointer to a packet parsing function
                                NULL //Termination
};
static RPC_LIST bcsIo_asyn_c={	BOWLER_CRIT,// Method
                                "asyn",//RPC as string
                                &bcsIoProcessor_c,//function pointer to a packet parsing function
                                NULL //Termination
};

//Namespace
static NAMESPACE_LIST bcsIo ={	ioNSName,// The string defining the namespace
                                NULL,// the first element in the RPC list
                                &bcsIoAsyncEventCallback,// async for this namespace
                                NULL// no initial elements to the other namesapce field.
};




static BOOL namespcaedAdded = FALSE;
NAMESPACE_LIST * get_bcsIoNamespace(){
	if(!namespcaedAdded){
		namespcaedAdded =TRUE;
		//Add the RPC structs to the namespace
		//GET
		addRpcToNamespace(&bcsIo,& bcsIo_gchm_g);
		addRpcToNamespace(&bcsIo,& bcsIo_gacm_g);
		addRpcToNamespace(&bcsIo,& bcsIo_gchv_g);
		addRpcToNamespace(&bcsIo,& bcsIo_gacv_g);
		addRpcToNamespace(&bcsIo,& bcsIo_asyn_g);
		addRpcToNamespace(&bcsIo,& bcsIo_gchc_g);
		addRpcToNamespace(&bcsIo,& bcsIo_gcml_g);
		//POST
		addRpcToNamespace(&bcsIo,& bcsIo_schv_p);
		addRpcToNamespace(&bcsIo,& bcsIo_sacv_p);
		addRpcToNamespace(&bcsIo,& bcsIo_asyn_p);
		//CRIT
		addRpcToNamespace(&bcsIo,& bcsIo_cchn_c);
		addRpcToNamespace(&bcsIo,& bcsIo_schv_c);
		addRpcToNamespace(&bcsIo,& bcsIo_asyn_c);

		//IO stack initilizations
		initAdvancedAsync();


	}

	return &bcsIo;//Return pointer to the struct
}

