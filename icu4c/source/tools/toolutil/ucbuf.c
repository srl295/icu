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
#include "cmemory.h"
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
    int32_t remaining;
    FileStream* in;
    UConverter* conv;
    UBool showWarning; /* makes this API not produce any errors */
};

static UBool ucbuf_autodetect_nrw(FileStream* in, const char** cp,int* numRead){
    /* initial 0xa5 bytes: make sure that if we read <4 bytes we don't misdetect something */
    char start[4]={ '\xa5', '\xa5', '\xa5', '\xa5' };
    int cap =T_FileStream_size(in);
    UBool autodetect;
    int signatureLength;

    *numRead=0;
    *cp="";

    if(cap<=0) {
        return FALSE;
    }

    autodetect = TRUE;
    *numRead=T_FileStream_read(in, start, 4); /* *numRead might be <4 */
    if(start[0] == '\xFE' && start[1] == '\xFF') {
        *cp = "UTF-16BE";
        signatureLength=2;
    } else if(start[0] == '\xFF' && start[1] == '\xFE') {
        if(start[2] == '\x00' && start[3] =='\x00'){
            *cp="UTF-32LE";
            signatureLength=4;
        } else {
            *cp = "UTF-16LE";
            signatureLength=2;
        }
    } else if(start[0] == '\xEF' && start[1] == '\xBB' && start[2] == '\xBF') {
        *cp = "UTF-8";
        signatureLength=3;
    }else if(start[0] == '\x0E' && start[1] == '\xFE' && start[2] == '\xFF'){
        *cp ="SCSU";
        signatureLength=3;
    }else if(start[0] == '\x00' && start[1] == '\x00' &&
            start[2] == '\xFE' && start[3]=='\xFF'){
        *cp = "UTF-32BE";
        signatureLength=4;
    }else{
        signatureLength=0;
        autodetect=FALSE;
    }
/*    T_FileStream_rewind(in);
    T_FileStream_read(in, start, *numRead);*/
    while(signatureLength<*numRead) {
        T_FileStream_ungetc(start[--*numRead], in);
    }
    return autodetect;
}

/* Autodetects UTF8, UTF-16-BigEndian and UTF-16-LittleEndian BOMs*/
U_CAPI UBool U_EXPORT2
ucbuf_autodetect(FileStream* in,const char** cp){
    UBool autodetect = FALSE;
    int numRead =0;
    const char* tcp;
    autodetect=ucbuf_autodetect_nrw(in,&tcp, &numRead);
    *cp =tcp;
    /* rewind the file Stream */
    T_FileStream_rewind(in);
    return autodetect;
}

/* fill the uchar buffer */
static UCHARBUF*
ucbuf_fillucbuf( UCHARBUF* buf,UErrorCode* err){
    UChar* pTarget=NULL;
    UChar* target=NULL;
    const char* source=NULL;
    char  cbuf[MAX_IN_BUF] = {'\0'};
    int32_t numRead=0;
    int32_t offset=0;
    const char* sourceLimit =NULL;
    pTarget = buf->buffer;
    /* check if we arrived here without exhausting the buffer*/
    if(buf->currentPos<buf->bufLimit){
        offset = (int32_t)(buf->bufLimit-buf->currentPos);
        memmove(buf->buffer,buf->currentPos,offset* sizeof(UChar));
    }

#if DEBUG
    memset(pTarget+offset,0xff,sizeof(UChar)*(MAX_IN_BUF-offset));
#endif

    /* read the file */
    numRead=T_FileStream_read(buf->in,cbuf,MAX_IN_BUF-offset);
    buf->remaining-=numRead;

    /* just to be sure...*/
    if ( 0 == numRead )
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
           err);
        /* since state is saved in the converter we add offset to source*/
        target = pTarget+offset;
        source = cbuf;
        sourceLimit = source + numRead;
        ucnv_toUnicode(buf->conv,&target,target+(MAX_U_BUF-offset),
                        &source,source+numRead,NULL,
                        (UBool)(buf->remaining==0),err);

        if(U_FAILURE(*err)){
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
                               u_errorName(*err));
            }

            *err = U_ZERO_ERROR;

            /* now get the context chars */
            ucnv_getInvalidChars(buf->conv,context,&len,err);
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
               err);

            /* reset source and target start positions */
            target = pTarget+offset;
            source = cbuf;

            /* re convert */
            ucnv_toUnicode(buf->conv,&target,target+(MAX_U_BUF-offset),
                            &source,sourceLimit,NULL,
                            (UBool)(buf->remaining==0),err);

        }
        numRead = (int32_t)(target - pTarget);


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
        u_charsToUChars(cbuf,target+offset,numRead);
        numRead=((buf->remaining>MAX_IN_BUF)? MAX_IN_BUF:numRead+offset);
    }
    buf->currentPos = pTarget;
    buf->bufLimit=pTarget+numRead;
    return buf;
}

/* get a UChar from the stream*/
U_CAPI UChar32 U_EXPORT2
ucbuf_getc(UCHARBUF* buf,UErrorCode* err){
    if(buf->currentPos>=buf->bufLimit){
        if(buf->remaining==0){
            return U_EOF;
        }
        buf=ucbuf_fillucbuf(buf,err);
        if(U_FAILURE(*err)){
            return U_EOF;
        }
    }

    return *(buf->currentPos++);
}


/* u_unescapeAt() callback to return a UChar*/
static UChar
_charAt(int32_t offset, void *context) {
    return ((UCHARBUF*) context)->currentPos[offset];
}

/* getc and escape it */
U_CAPI UChar32 U_EXPORT2
ucbuf_getcx(UCHARBUF* buf,UErrorCode* err) {
    int32_t length;
    int32_t offset;
    UChar32 c32,c1,c2;

    /* Fill the buffer if it is empty */
    if (buf->currentPos >=buf->bufLimit-2) {
        ucbuf_fillucbuf(buf,err);
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
        ucbuf_fillucbuf(buf,err);
        length = (int32_t)(buf->bufLimit - buf->buffer);
    }

    /* Process the escape */
    offset = 0;
    c32 = u_unescapeAt(_charAt, &offset, length, (void*)buf);

    /* check if u_unescapeAt unescaped and converted
     * to c32 or not
     */
    if(c32!=c2){
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

/* open a UCHARBUF */
U_CAPI UCHARBUF* U_EXPORT2
ucbuf_open(FileStream* in,const char* cp, UBool showWarning, UErrorCode* err){

    UCHARBUF* buf =(UCHARBUF*) uprv_malloc(sizeof(UCHARBUF));
    int numRead =0;
    if(U_FAILURE(*err)){
        return NULL;
    }
    if(buf){
        buf->in=in;
        buf->conv=NULL;
        buf->showWarning = showWarning;
        if(!cp ||(cp && *cp=='\0')){
            /* don't have code page name... try to autodetect */
            if(ucbuf_autodetect_nrw(in,&cp,&numRead)){
                buf->conv=ucnv_open(cp,err);
            }
        }else{
            buf->conv=ucnv_open(cp,err);
        }

        if((buf->conv==NULL) && (buf->showWarning==TRUE)){
            fprintf(stderr,"###WARNING: No converter defined. Using codepage of system.\n");
        }
        buf->remaining=T_FileStream_size(in)-numRead;
        buf->buffer=(UChar*) uprv_malloc(sizeof(UChar)* MAX_U_BUF);
        if (buf->buffer == NULL) {
            *err = U_MEMORY_ALLOCATION_ERROR;
            return NULL;
        }
        buf->currentPos=buf->buffer;
        buf->bufLimit=buf->buffer;
        if(U_FAILURE(*err)){
            fprintf(stderr, "Could not open codepage [%s]: %s\n", cp, u_errorName(*err));
            return NULL;
        }
        buf=ucbuf_fillucbuf(buf,err);
        return buf;
    }else{
        *err = U_MEMORY_ALLOCATION_ERROR;
        return NULL;
    }
}

/* TODO: this method will fail if at the
 * begining of buffer and the uchar to unget
 * is from the previous buffer. Need to implement
 * system to take care of that situation.
 */
U_CAPI void U_EXPORT2
ucbuf_ungetc(UChar32 c,UCHARBUF* buf){
    /* decrement currentPos pointer
     * if not at the begining of buffer
     */
    if(buf->currentPos!=buf->buffer){
        buf->currentPos--;
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
    if(buf->conv){
        ucnv_close(buf->conv);
    }
    buf->in=NULL;
    buf->currentPos=NULL;
    buf->bufLimit=NULL;
    ucbuf_closebuf(buf);
    uprv_free(buf);
}

/* rewind the buf and file stream */
U_CAPI void U_EXPORT2
ucbuf_rewind(UCHARBUF* buf){
    if(buf){
        const char* cp="";
        buf->currentPos=buf->buffer;
        buf->bufLimit=buf->buffer;
        ucnv_reset(buf->conv);
        T_FileStream_rewind(buf->in);
        ucbuf_autodetect(buf->in,&cp);
        buf->remaining=T_FileStream_size(buf->in);
    }
}
