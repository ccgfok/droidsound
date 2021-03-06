
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <jni.h>
#include <android/log.h>

extern "C"
{
	#include "sexypsf/driver.h"
}

#include "com_ssb_droidsound_plugins_SexyPSFPlugin.h"

#define INFO_TITLE 0
#define INFO_AUTHOR 1
#define INFO_LENGTH 2
#define INFO_TYPE 3
#define INFO_COPYRIGHT 4
#define INFO_GAME 5
#define INFO_SUBTUNES 6
#define INFO_STARTTUNE 7
#define INFO_SUBTUNE_TITLE 8

unsigned char *outBuffer;
unsigned char *outPtr;

static jstring NewString(JNIEnv *env, const char *str)
{
	static jchar *temp, *ptr;

	temp = (jchar *) malloc((strlen(str) + 1) * sizeof(jchar));
	ptr = temp;
    
	while(*str)
    {
		unsigned char c = (unsigned char)*str++;
		*ptr++ = (c < 0x7f && c >= 0x20) || c >= 0xa0 || c == 0xa ? c : '?';
	}

	jstring j = env->NewString(temp, ptr - temp);

	free(temp);

	return j;
}

static bool playing = false;

void sexyd_update(unsigned char *pSound, long lBytes)
{
	//__android_log_print(ANDROID_LOG_VERBOSE, "SexyPSF", "update with %d bytes", lBytes);
	memcpy(outPtr, pSound, lBytes);
	outPtr += lBytes;
}


JNIEXPORT jlong JNICALL Java_com_ssb_droidsound_plugins_SexyPSFPlugin_N_1loadFile(JNIEnv *env, jobject obj, jstring fname)
{
	const char *filename = env->GetStringUTFChars(fname, NULL);
	__android_log_print(ANDROID_LOG_VERBOSE, "SexyPSF", "Trying to load '%s'", filename);

	playing = false;

	if(outBuffer == NULL) {
		outBuffer = (unsigned char *)malloc(1024 * 1024);
		outPtr = outBuffer;
	}

	char temp[1024];
	strcpy(temp, filename);

	PSFINFO *psfInfo = sexy_load(temp);
	__android_log_print(ANDROID_LOG_VERBOSE, "SexyPSF", "Got %p", psfInfo);
	return (long)psfInfo;

}

JNIEXPORT void JNICALL Java_com_ssb_droidsound_plugins_SexyPSFPlugin_N_1unload(JNIEnv *env, jobject obj, jlong song)
{
	__android_log_print(ANDROID_LOG_VERBOSE, "SexyPSF", "Unload while %splaying", playing ? "" : "NOT ");

	playing = false;

	if(outBuffer != NULL) {
		free(outBuffer);
		outBuffer = NULL;
	}

	PSFINFO *psfInfo = (PSFINFO*)song;
	sexy_freepsfinfo(psfInfo);
	sexy_shutdown();
}

JNIEXPORT jint JNICALL Java_com_ssb_droidsound_plugins_SexyPSFPlugin_N_1getSoundData(JNIEnv *env, jobject obj, jlong song, jshortArray sArray, jint size)
{
	playing = true;

	while(outPtr - outBuffer < size*2) {
		int rc = sexy_execute();
		//__android_log_print(ANDROID_LOG_VERBOSE, "SexyPSF", "Execute:%d", rc);
		if(rc <= 0)
			return rc;
	}

	if(outBuffer == NULL) {
		return -1;
	}

	if(outPtr == outBuffer)
		return 0;

	jshort *dest = env->GetShortArrayElements(sArray, NULL);
	int bytelen = size*2;

	int filled = outPtr - outBuffer;
	if(bytelen > filled)
		bytelen = filled;

	//__android_log_print(ANDROID_LOG_VERBOSE, "SexyPSF", "Copying %d bytes sound", bytelen);
	memcpy(dest, outBuffer, bytelen);
	//__android_log_print(ANDROID_LOG_VERBOSE, "SexyPSF", "Copying %d bytes from offset %d", filled - bytelen, bytelen);
	if(filled > bytelen)
		memmove(outBuffer, &outBuffer[bytelen], filled - bytelen);
	outPtr = &outBuffer[filled - bytelen];

	env->ReleaseShortArrayElements(sArray, dest, 0);

	return bytelen / 2;
}


JNIEXPORT jstring JNICALL Java_com_ssb_droidsound_plugins_SexyPSFPlugin_N_1getStringInfo(JNIEnv *env, jobject obj, jlong song, jint what)
{
	PSFINFO *info = (PSFINFO*)song;

	switch(what)
    {
        case INFO_AUTHOR:
            return NewString(env, info->artist ? info->artist : "");
        case INFO_TITLE:
            return NewString(env, info->title ? info->title : "");
        case INFO_COPYRIGHT:
            return NewString(env, info->copyright ? info->copyright : "");
        case INFO_GAME:
            return NewString(env, info->game ? info->game : "");
	}
    
	return NULL;
}

JNIEXPORT jint JNICALL Java_com_ssb_droidsound_plugins_SexyPSFPlugin_N_1getIntInfo(JNIEnv *env, jobject obj, jlong song, jint what)
{
	PSFINFO *info = (PSFINFO*)song;
		switch(what)
        {
            case INFO_LENGTH:
                return info->length;
		}
		return 0;
}
