1. Description of different types of libraries.
	liboasis_lite2D_DEFAULT_106f_ae.a, default OASIS LITE library build for RT106F MCU, support RGB+IR camera module.
	liboasis_lite2D_DEFAULT_android_reg.a, default OASIS LITE library build for Android platform, support only face detection
    and face registration, no antispoofing buildin. It is designed for remote registration purpose.
	liboasis_lite2D_DEFAULT_linux_reg.a, default OASIS LITE library build for Linux 64 bit platform, support only face detection
    and face registration, no antispoofing buildin. It is designed for remote registration purpose.
	
2. How to hook customized algorithm into OASIS LITE library.
	OASIS LITE library support to hook customized algorithmes. For the time being,face recognition and liveness check algorithm hook interfaces
    has been implemented, other algorithm hook interfaces are under development.
    2.1 Hook customized face recognition algorithm
	a) Generate NanoAi format header file by NanoAi toolchain(provided by NXP AI Iot solution team).
        b) Copy the header file into the same directory with "algorithm_port.c" file and rename header file's name 
         to "face_rec_model.h".
        c) Modify definition of macroes according the define in header file:
			FACE_REC_NANOAI_MODEL_INSTANCE 
			FACE_REC_NANOAI_MODEL_DATA_INSTANCE 
			FACE_REC_NANOAI_MODEL_OUTPUT_ID
                        
	d) Define input and output dimention parameters
			FACE_REC_INPUT_C 3  // only support 3 channels currently
			FACE_REC_INPUT_H 112  
			FACE_REC_INPUT_W 112
			FACE_REC_OUTPUT_C 512
                        FACE_REC_THRESHOLD (0.7f)
	e) Enable customized face recognition algorithm
		  #define CUSTOMIZE_FACE_REC_ALGO 1
     2.2 Hook customized liveness check algorithm on IR frame
	a) Generate NanoAi format header file by NanoAi toolchain(provided by NXP AI Iot solution team).
        b) Copy the header file into the same directory with "algorithm_port.c" file and rename header file's name 
         to "liveness_model.h".
        c) Modify definition of macroes according the define in header file:
			LIVENESS_NANOAI_MODEL_INSTANCE 
			LIVENESS_NANOAI_MODEL_DATA_INSTANCE 
			LIVENESS_NANOAI_MODEL_OUTPUT_ID
	d) Define input and threshold dimention parameters
			LIVENESS_INPUT_C 3  //support 1 or 3 channels currently
			LIVENESS_INPUT_H 112  
			LIVENESS_INPUT_W 112
			LIVENESS_THRESHOLD (0.5f)
	e) Enable customized face recognition algorithm
		  #define CUSTOMIZE_LIVENESS_ALGO 1
        
     2.3 Rebuild and customized algorithm will be embedded into the firmware.
    
	 
