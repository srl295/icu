#include "unicode/utypes.h"
#include "unicode/ucnv.h"
#include "flagcb.h"

U_CAPI void U_EXPORT2 UCNV_FROM_U_CALLBACK_FLAG (
                  const void *context,
                  UConverterFromUnicodeArgs *fromUArgs,
                  const UChar* codeUnits,
                  int32_t length,
                  UChar32 codePoint,
                  UConverterCallbackReason reason,
				  UErrorCode * err)
{
  /* First step - based on the reason code, take action */

  if(reason == UCNV_UNASSIGNED) { /* whatever set should be trapped here */
    ((FromUFLAGContext*)context)->flag = TRUE;
  }

  if(reason == UCNV_CLONE) {
    
  }

  /* Always call the subCallback if present */
  
  if(((FromUFLAGContext*)context)->subCallback != NULL)
    {
        ((FromUFLAGContext*)context)->subCallback(  ((FromUFLAGContext*)context)->subContext,
                                                    fromUArgs,
                                                    codeUnits,
                                                    length,
                                                    codePoint,
                                                    reason,
                                                    err);

    }

  /* cleanup */
  if(reason == UCNV_CLOSE) {
    free(context);
  }



}
