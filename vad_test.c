#include<stdio.h>
#include "vad.h"
int main()
{
	FILE *fpInputf;
	int ret = 0;
	int noVoiceCount = 0;
	int maxZeroFrame = 80; // 连续80帧（delay = 80 * 采样/帧长）检测到没有声音  标识停止
	int voiceStartFlag = 0; // 是否标志了语音开始 0 未标定
	int kRates[] = { 8000, 12000, 16000, 24000, 32000, 48000 };
	int kFrameLengths[] = { 80, 120, 160, 240, 320, 480, 640, 960 };
	short datas[kFrameLengths[2]];
		
	VadInst* handle = NULL;
	int mode = 2; // 模式
	// 1.初始化 设置模式
	WebRtcVad_Create(&handle);
	WebRtcVad_Init(handle);
	WebRtcVad_set_mode(handle,mode);

	fpInputf=fopen("deb_01.wav","rb");
	if(fpInputf==NULL)
	{
		printf("There is no input file\n");
		return 0;
	}

	while(!feof(fpInputf))
	{
		fread(datas,sizeof(short),kFrameLengths[2],fpInputf);
		// 2.执行检测
		ret = vad_main_process(handle, kFrameLengths[2], kRates[0], datas);
		
		if(ret == 0) {
			noVoiceCount+=1;
			if(noVoiceCount >= maxZeroFrame){ 
				if(voiceStartFlag == 1){ // 标记语音停止
					voiceStartFlag = 0;
					// ...
				}
				noVoiceCount = 0;
			}
		} else { // 标记语音开始
			// ...
			voiceStartFlag = 1;
			noVoiceCount = 0;
		}
		
	}
	fclose(fpInputf);	
	// 3.释放
	WebRtcVad_Free(handle);
	
	printf("finished \n");
	return 0;
}
