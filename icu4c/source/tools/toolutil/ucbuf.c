/*
*******************************************************************************
*
*   Copyright (C) 1998-2001, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*
* File ucbuf.c
*
* Modification History:
*
*   Date        Name        Description
*   05/10/01    Ram         Creation.
*******************************************************************************
*/

#include "unicode/utypes.h"
#include "unicode/ucnv.h"
#include "unicode/ucnv_err.h"
#include "filestrm.h"
#include "cstring.h"
#include "cmemory.h"
#include "ustrfmt.h"
#include "unicode/ustring.h"
#include "ucbuf.h"
#include <stdio.h>

#define MAX_IN_BUF 1000
#define MAX_U_BUF 1500
#define CONTEXT_LEN 15

struct UCHARBUF {
    UChar* buffer;
    UChar* currentPos;
    UChar* bufLimit;
    int32_t bufCapacity;
    int32_t remaining;
    int32_t signatureLength;
    FileStream* in;
    UConverter* conv;
    UBool showWarning; /* makes this API not produce any errors */
    UBool isBuffered;
};

U_CAPI UBool U_EXPORT2
ucbuf_autodetect_fs(FileStream* in, const char** cp, UConverter** conv, int32_t* signatureLength, UErrorCode* error){
    char start[8];
    int32_t numRead;

    UChar target[1]={ 0 };
    UChar* pTarget;
    const char* pStart;

    /* read a few bytes */
    numRead=T_FileStream_read(in, start, sizeof(start));

    *cp = ucnv_detectUnicodeSignature(start, numRead, signatureLength, error);
    if(*cp==NULL){
        /* unread the bytes already read */
        while(numRead>0) {
            T_FileStream_ungetc(start[--numRead], in);
        }

        *conv =NULL;
        return FALSE;
    }

    /* open the converter for the detected Unicode charset */
    *conv = ucnv_open(*cp,error);

    /* convert and ignore initial U+FEFF, and the buffer overflow */
    pTarget = target;
    pStart = start;
    ucnv_toUnicode(*conv, &pTarget, target+1, &pStart, start+*signatureLength, NULL, FALSE, error);
    *signatureLength = pStart - start;
    if(*error==U_BUFFER_OVERFLOW_ERROR) {
        *error=U_ZERO_ERROR;
    }

    /* verify that we successfully read exactly U+FEFF */
    if(U_SUCCESS(*error) && (pTarget!=(target+1) || target[0]!=0xfeff)) {
        *error=U_INTERNAL_PROGRAM_ERROR;
    }

    /* unread the bytes beyond what was consumed for U+FEFF */
    while(numRead>*signatureLength) {
        T_FileStream_ungetc(start[--numRead], in);
    }

    return TRUE; 
}

U_CAPI FileStream * U_EXPORT2
ucbuf_autodetect(const char* fileName, const char** cp,UConverter** conv, int32_t* signatureLength,UErrorCode* error){
    FileStream* in=NULL;
    if(error==NULL || U_FAILURE(*error)){
        return NULL;
    }
    if(conv==NULL || cp==NULL || fileName==NULL){
        *error = U_ILLEGAL_ARGUMENT_ERROR;
        return NULL;
    }
    /* open the file */
    in= T_FileStream_open(fileName,"rb");
    
    if(in == NULL){
        *error=U_FILE_ACCESS_ERROR;
        return NULL;
    }

    if(ucbuf_autodetect_fs(in,cp,conv,signatureLength,error)) {
        return in;
    } else {
        ucnv_close(*conv);
        *conv=NULL;
        T_FileStream_close(in);
        return NULL;
    }
}

/* fill the uchar buffer */
static UCHARBUF*
ucbuf_fillucbuf( UCHARBUF* buf,UErrorCode* error){
    UChar* pTarget=NULL;
    UChar* target=NULL;
    const char* source=NULL;
    char  carr[MAX_IN_BUF] = {'\0'};
    char* cbuf =  carr;
    int32_t inputRead=0;
    int32_t outputWritten=0;
    int32_t offset=0;
    const char* sourceLimit =NULL;
    int32_t cbufSize=0;
    pTarget = buf->buffer;
    /* check if we arrived here without exhausting the buffer*/
    if(buf->currentPos<buf->bufLimit){
        offset = (int32_t)(buf->bufLimit-buf->currentPos);
        memmove(buf->buffer,buf->currentPos,offset* sizeof(UChar));
    }

#if DEBUG
    memset(pTarget+offset,0xff,sizeof(UChar)*(MAX_IN_BUF-offset));
#endif
    if(buf->isBuffered){
        cbufSize = MAX_IN_BUF;
        /* read the file */
        inputRead=T_FileStream_read(buf->in,cbuf,cbufSize-offset);
        buf->remaining-=inputRead;
        
    }else{
        cbufSize = T_FileStream_size(buf->in);
        cbuf = (char*)uprv_malloc(cbufSize);
        inputRead= T_FileStream_read(buf->in,cbuf,cbufSize);
        buf->remaining-=inputRead;
    }

    /* just to be sure...*/
    if ( 0 == inputRead )
       buf->remaining = 0;

    target=pTarget;
    /* convert the bytes */
    if(buf->conv){
        /* set the callback to stop */
        UConverterToUCallback toUOldAction ;
        void* toUOldContext;
        void* toUNewContext=NULL;
        ucnv_setToUCallBack(buf->conv,
           UCNV_TO_U_CALLBACK_STOP,
           toUNewContext,
           &toUOldAction,
           (const void**)&toUOldContext,
           error);
        /* since state is saved in the converter we add offset to source*/
        target = pTarget+offset;
        source = cbuf;
        sourceLimit = source + inputRead;
        ucnv_toUnicode(buf->conv,&target,target+(buf->bufCapacity-offset),
                        &source,sourceLimit,NULL,
                        (UBool)(buf->remaining==0),error);

        if(U_FAILURE(*error)){
            char context[CONTEXT_LEN];
            char preContext[CONTEXT_LEN];
            char postContext[CONTEXT_LEN];
            int8_t len = CONTEXT_LEN;
            int32_t start=0;
            int32_t stop =0;
            int32_t pos =0;

            if( buf->showWarning==TRUE){
                fprintf(stderr,"\n###WARNING: Encountered abnormal bytes while"
                               " converting input stream to target encoding: %s\n",
                               u_errorName(*error));
            }

            *error = U_ZERO_ERROR;

            /* now get the context chars */
            ucnv_getInvalidChars(buf->conv,context,&len,error);
            context[len]= 0 ; /* null terminate the buffer */

            pos = (int32_t)(source - cbuf - len);

            /* for pre-context */
            start = (pos <=CONTEXT_LEN)? 0 : (pos - (CONTEXT_LEN-1));
            stop  = pos-len;

            memcpy(preContext,cbuf+start,stop-start);
            /* null terminate the buffer */
            preContext[stop-start] = 0;

            /* for post-context */
            start = pos+len;
            stop  = (int32_t)(((pos+CONTEXT_LEN)<= (sourceLimit-cbuf) )? (pos+(CONTEXT_LEN-1)) : (sourceLimit-cbuf));

            memcpy(postContext,source,stop-start);
            /* null terminate the buffer */
            postContext[stop-start] = 0;

            if(buf->showWarning ==TRUE){
                /* print out the context */
                fprintf(stderr,"\tPre-context: %s\n",preContext);
                fprintf(stderr,"\tContext: %s\n",context);
                fprintf(stderr,"\tPost-context: %s\n", postContext);
            }

            /* reset the converter */
            ucnv_reset(buf->conv);

            /* set the call back to substitute
             * and restart conversion
             */
            ucnv_setToUCallBack(buf->conv,
               UCNV_TO_U_CALLBACK_SUBSTITUTE,
               toUNewContext,
               &toUOldAction,
               (const void**)&toUOldContext,
               error);

            /* reset source and target start positions */
            target = pTarget+offset;
            source = cbuf;

            /* re convert */
            ucnv_toUnicode(buf->conv,&target,target+(buf->bufCapacity-offset),
                            &source,sourceLimit,NULL,
                            (UBool)(buf->remaining==0),error);

        }
        outputWritten = (int32_t)(target - pTarget);


#if DEBUG
        {
            int i;
            target = pTarget;
            for(i=0;i<numRead;i++){
              /*  printf("%c", (char)(*target++));*/
            }
        }
#endif

    }else{
        u_charsToUChars(cbuf,target+offset,inputRead);
        outputWritten=((buf->remaining>cbufSize)? cbufSize:inputRead+offset);
    }
    buf->currentPos = pTarget;
    buf->bufLimit=pTarget+outputWritten;
    if(cbuf!=carr){
        uprv_free(cbuf);
    }
    return buf;
}



/* get a UChar from the stream*/
U_CAPI int32_t U_EXPORT2
ucbuf_getc(UCHARBUF* buf,UErrorCode* error){
    if(error==NULL || U_FAILURE(*error)){
        return FALSE;
    }
    if(buf->currentPos>=buf->bufLimit){
        if(buf->remaining==0){
            return U_EOF;
        }
        buf=ucbuf_fillucbuf(buf,error);
        if(U_FAILURE(*error)){
            return U_EOF;
        }
    }

    return *(buf->currentPos++);
}

/* get a UChar32 from the stream*/
U_CAPI int32_t U_EXPORT2
ucbuf_getc32(UCHARBUF* buf,UErrorCode* error){
    int32_t retVal =U_EOF;
    if(error==NULL || U_FAILURE(*error)){
        return FALSE;
    }
    if(buf->currentPos+1>=buf->bufLimit){
        if(buf->remaining==0){
            return U_EOF;
        }
        buf=ucbuf_fillucbuf(buf,error);
        if(U_FAILURE(*error)){
            return U_EOF;
        }
    }
    if(UTF_IS_LEAD(*(buf->currentPos))){
        retVal=UTF16_GET_PAIR_VALUE(*(buf->currentPos++),*(buf->currentPos++));
    }else{
        retVal = *(buf->currentPos++);
    }
    return retVal;
}

/* u_unescapeAt() callback to return a UChar*/
static UChar U_CALLCONV
_charAt(int32_t offset, void *context) {
    return ((UCHARBUF*) context)->currentPos[offset];
}

/* getc and escape it */
U_CAPI int32_t U_EXPORT2
ucbuf_getcx32(UCHARBUF* buf,UErrorCode* error) {
    int32_t length;
    int32_t offset;
    UChar32 c32,c1,c2;
    if(error==NULL || U_FAILURE(*error)){
        return FALSE;
    }
    /* Fill the buffer if it is empty */
    if (buf->currentPos >=buf->bufLimit-2) {
        ucbuf_fillucbuf(buf,error);
    }

    /* Get the next character in the buffer */
    if (buf->currentPos < buf->bufLimit) {
        c1 = *(buf->currentPos)++;
    } else {
        c1 = U_EOF;
    }

    c2 = *(buf->currentPos);

    /* If it isn't a backslash, return it */
    if (c1 != 0x005C) {
        return c1;
    }

    /* Determine the amount of data in the buffer */
    length = (int32_t)(buf->bufLimit - buf->currentPos);

    /* The longest escape sequence is \Uhhhhhhhh; make sure
       we have at least that many characters */
    if (length < 10) {

        /* fill the buffer */
        ucbuf_fillucbuf(buf,error);
        length = (int32_t)(buf->bufLimit - buf->buffer);
    }

    /* Process the escape */
    offset = 0;
    c32 = u_unescapeAt(_charAt, &offset, length, (void*)buf);

    /* check if u_unescapeAt unescaped and converted
     * to c32 or not
     */
    if(c32==0xFFFFFFFF){
        *error= U_ILLEGAL_ESCAPE_SEQUENCE;
        return c1;
    }else if(c32!=c2){
        /* Update the current buffer position */
        buf->currentPos += offset;
    }else{
        /* unescaping failed so we just return
         * c1 and not consume the buffer
         * this is useful for rules with escapes
         * in resouce bundles
         * eg: \' \\ \"
         */
        return c1;
    }

    return c32;
}

U_CAPI UCHARBUF* U_EXPORT2
ucbuf_open(const char* fileName,const char** cp,UBool showWarning, UBool buffered, UErrorCode* error){

    FileStream* in = NULL; 
    int32_t fileSize=0;
    if(error==NULL || U_FAILURE(*error)){
        return NULL;
    }
    if(cp==NULL || fileName==NULL){
        *error = U_ILLEGAL_ARGUMENT_ERROR;
        return FALSE;
    }
    if (!uprv_strcmp(fileName, "-")) {
        in = T_FileStream_stdin();
    }else{ 
        in = T_FileStream_open(fileName, "rb");
    }
    
    if(in!=NULL){
        UCHARBUF* buf =(UCHARBUF*) uprv_malloc(sizeof(UCHARBUF));
        fileSize = T_FileStream_size(in);
        if(buf){
            buf->in=in;
            buf->conv=NULL;
            buf->showWarning = showWarning;
            buf->isBuffered = buffered;
            if(*cp==NULL || **cp=='\0'){
                /* don't have code page name... try to autodetect */
                ucbuf_autodetect_fs(in,cp,&buf->conv,&buf->signatureLength,error);
            }
            if(U_SUCCESS(*error) && buf->conv==NULL) {
                buf->conv=ucnv_open(*cp,error);
            }
            if(U_FAILURE(*error)){
                ucnv_close(buf->conv);
                uprv_free(buf);
                return NULL;
            }
            
            if((buf->conv==NULL) && (buf->showWarning==TRUE)){
                fprintf(stderr,"###WARNING: No converter defined. Using codepage of system.\n");
            }
            buf->remaining=fileSize-buf->signatureLength;
            if(buf->isBuffered){
                buf->buffer=(UChar*) uprv_malloc(U_SIZEOF_UCHAR* MAX_U_BUF);
                buf->bufCapacity=MAX_U_BUF;
            }else{
                buf->buffer=(UChar*) uprv_malloc(U_SIZEOF_UCHAR * (buf->remaining+buf->signatureLength));
                buf->bufCapacity=buf->remaining+buf->signatureLength;
            }
            if (buf->buffer == NULL) {
                *error = U_MEMORY_ALLOCATION_ERROR;
                return NULL;
            }
            buf->currentPos=buf->buffer;
            buf->bufLimit=buf->buffer;
            if(U_FAILURE(*error)){
                fprintf(stderr, "Could not open codepage [%s]: %s\n", *cp, u_errorName(*error));
                return NULL;
            }
            buf=ucbuf_fillucbuf(buf,error);
            return buf;
        }else{
            *error = U_MEMORY_ALLOCATION_ERROR;
            return NULL;
        }

    }
    *error =U_FILE_ACCESS_ERROR;
    return NULL;
}



/* TODO: this method will fail if at the
 * begining of buffer and the uchar to unget
 * is from the previous buffer. Need to implement
 * system to take care of that situation.
 */
U_CAPI void U_EXPORT2
ucbuf_ungetc(int32_t c,UCHARBUF* buf){
    /* decrement currentPos pointer
     * if not at the begining of buffer
     */
    UChar escaped[8] ={'\0'};
    int32_t len =0;
    if(c > 0xFFFF){
        len = uprv_itou(escaped,c,16,8);
    }else{
        len=uprv_itou(escaped,c,16,4);
    }
    if(buf->currentPos!=buf->buffer){
        if(*(buf->currentPos-1)==c){
            buf->currentPos--;
        }else if(u_strncmp(buf->currentPos-len,escaped,len) == 0){
            while(--len>0){
                buf->currentPos--;
            }
        }
    }
}

/* frees the resources of UChar* buffer */
static void
ucbuf_closebuf(UCHARBUF* buf){
    uprv_free(buf->buffer);
    buf->buffer = NULL;
}

/* close the buf and release resources*/
U_CAPI void U_EXPORT2
ucbuf_close(UCHARBUF* buf){
    if(buf!=NULL){
        if(buf->conv){
            ucnv_close(buf->conv);
        }
        buf->in=NULL;
        buf->currentPos=NULL;
        buf->bufLimit=NULL;
        T_FileStream_close(buf->in);
        ucbuf_closebuf(buf);
        uprv_free(buf);
    }
}

/* rewind the buf and file stream */
U_CAPI void U_EXPORT2
ucbuf_rewind(UCHARBUF* buf,UErrorCode* error){
    if(error==NULL || U_FAILURE(*error)){
        return;
    }
    if(buf){
        buf->currentPos=buf->buffer;
        buf->bufLimit=buf->buffer;
        T_FileStream_rewind(buf->in);
        buf->remaining=T_FileStream_size(buf->in)-buf->signatureLength;

        ucnv_resetToUnicode(buf->conv);
        if(buf->signatureLength>0) {
            UChar target[1]={ 0 };
            UChar* pTarget;
            char start[8];
            const char* pStart;
            int32_t numRead;

            /* read the signature bytes */
            numRead=T_FileStream_read(buf->in, start, buf->signatureLength);

            /* convert and ignore initial U+FEFF, and the buffer overflow */
            pTarget = target;
            pStart = start;
            ucnv_toUnicode(buf->conv, &pTarget, target+1, &pStart, start+numRead, NULL, FALSE, error);
            if(*error==U_BUFFER_OVERFLOW_ERROR) {
                *error=U_ZERO_ERROR;
            }

            /* verify that we successfully read exactly U+FEFF */
            if(U_SUCCESS(*error) && (numRead!=buf->signatureLength || pTarget!=(target+1) || target[0]!=0xfeff)) {
                *error=U_INTERNAL_PROGRAM_ERROR;
            }
        }
    }
}


U_CAPI int32_t U_EXPORT2
ucbuf_size(UCHARBUF* buf){
    if(buf){
        if(buf->isBuffered){
            return (T_FileStream_size(buf->in)-buf->signatureLength)/ucnv_getMinCharSize(buf->conv);
        }else{
            return buf->bufLimit-buf->buffer;
        }
    }
    return 0;
}

U_CAPI const UChar* U_EXPORT2
ucbuf_getBuffer(UCHARBUF* buf,int32_t* len,UErrorCode* error){
    if(error==NULL || U_FAILURE(*error)){
        return NULL;
    }
    if(buf==NULL || len==NULL){
        *error = U_ILLEGAL_ARGUMENT_ERROR;
        return NULL;
    }
    *len = buf->bufLimit-buf->buffer;
    return buf->buffer;
}

U_CAPI const char* U_EXPORT2
ucbuf_resolveFileName(const char* inputDir, const char* fileName, char* target, int32_t* len, UErrorCode* status){
    int32_t requiredLen = 0;
    int32_t dirlen =  0;
    int32_t filelen = 0;
    if(status==NULL || U_FAILURE(*status)){
        return NULL;
    }

    if(inputDir == NULL || fileName == NULL || len==NULL || (target==NULL && *len>0)){
        *status = U_ILLEGAL_ARGUMENT_ERROR;
        return NULL;
    }


    dirlen  = (int32_t)uprv_strlen(inputDir);
    filelen = (int32_t)uprv_strlen(fileName);
    if(inputDir[dirlen-1] != U_FILE_SEP_CHAR) {
        requiredLen = dirlen + filelen + 2;
        if((*len < requiredLen) || target==NULL){
            *len = requiredLen;
            *status = U_BUFFER_OVERFLOW_ERROR;
            return NULL;
        }

        target[0] = '\0';
        /*
         * append the input dir to openFileName if the first char in 
         * filename is not file seperation char and the last char input directory is  not '.'.
         * This is to support :
         * genrb -s. /home/icu/data
         * genrb -s. icu/data
         * The user cannot mix notations like
         * genrb -s. /icu/data --- the absolute path specified. -s redundant
         * user should use
         * genrb -s. icu/data  --- start from CWD and look in icu/data dir
         */
        if( (fileName[0] != U_FILE_SEP_CHAR) && (inputDir[dirlen-1] !='.')){
            uprv_strcpy(target, inputDir);
            target[dirlen]     = U_FILE_SEP_CHAR;
        }
        target[dirlen + 1] = '\0';
    } else {
        requiredLen = dirlen + filelen + 1;
        if((*len < requiredLen) || target==NULL){
            *len = requiredLen;
            *status = U_BUFFER_OVERFLOW_ERROR;
            return NULL;
        }
        
        uprv_strcpy(target, inputDir);
    }

    uprv_strcat(target, fileName);
    return target;
}
U_CAPI const UChar* U_EXPORT2
ucbuf_readline(UCHARBUF* buf,int32_t* len,UErrorCode* err){
    UChar* temp = buf->currentPos;
    UChar* savePos =NULL;
    UChar c=0x0000;
    if(buf->isBuffered){
    /* The input is buffered we have to do more 
    * for returning a pointer U_TRUNCATED_CHAR_FOUND
        */
        for(;;){
            c = *temp++;
            /* Watch for CR, LF, EOF; these finish off a line.*/
            if (c == 0xd) {
                continue;
            }
            if(buf->remaining==0){
                *err = (UErrorCode) U_EOF;
            }
            if(temp>=buf->bufLimit && buf->currentPos == buf->buffer){
                *err= U_TRUNCATED_CHAR_FOUND;
                return NULL;
            }else{
                ucbuf_fillucbuf(buf,err);
                if(U_FAILURE(*err)){
                    return NULL;
                }
            }
            if (c == 0x0a || c==0x2028) {  /* Unipad inserts 2028 line separators! */
                *len = temp - buf->currentPos;
                savePos = buf->currentPos;
                buf->currentPos = temp;
                return savePos;
            }
        }
    }else{
    /* we know that all input is read into the internal
    * buffer so we can safely return pointers
        */
        for(;;){
            c = *temp++;
            /* Watch for CR, LF, EOF; these finish off a line.*/
            if (c == 0xd) {
                continue;
            }
            if(buf->currentPos==buf->bufLimit){
                *err = (UErrorCode) U_EOF;
                return NULL;
            }
            if (temp>=buf->bufLimit || c == 0x0a || c==0x2028) {  /* Unipad inserts 2028 line separators! */
                *len = temp - buf->currentPos;
                savePos = buf->currentPos;
                buf->currentPos = temp;
                return savePos;
            }
        }
    }
    return NULL;
}
