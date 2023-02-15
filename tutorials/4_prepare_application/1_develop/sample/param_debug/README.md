# Post Vision App sample for parameter debugging
The SDK provides sample code for debugging a Post Vision App.
This tutorial explains how to use the sample code to get debug information from the output of a Post Vision App.

## Overview
Debug information can be obtained by storing debug information in the variable that originally stores the inference result.<br>
By using this function, it is possible to obtain information for unexpected behavior at the time of inference execution such as not obtaining inference results after deploying a Post Vision App to an edge AI device.

## Getting started
The sample code provided is for object detection.
See below for sample code.
- [ppl_objectdetection.cpp](./ppl_objectdetection.cpp)

### Create a Post Vision App for debugging
#### 1. Implement debug code
When implementing debug code, store the debug information in the variable **`pp_out_buf`**, which is an argument of **`PPL_Analyze`**.

In the sample code, the debug information is obtained from **`PPL_ParseParam`** as **`EPPL_RESULT_CODE`** and stored in the variable **`pp_out_buf`**.

The sample code is implemented as follows :

```cpp
    if (!s_is_param_parsed) {
        ret = PPL_ParseParam();
        if (ret != E_PPL_OK) {
            s_err_buf = ret;
            *pp_out_buf = &s_err_buf;
            *p_out_size = 4;
            *p_upload_flag = true;
            return E_PPL_OK;
        }
        free((void *)sp_param);
        sp_param = NULL;
        s_is_param_parsed = true;
    }
```

> **NOTE**
>
> Debug information can be output by following the state transition rules in [PPL Library API Specification for IMX500](https://developer.aitrios.sony-semicon.com/development-guides/documents/specifications/) and returning **`E_PPL_OK`** for **`PPL_Analyze`**.

#### 2. Build
See [README for post process](../../README.md#2-build) to build the debug code.

### Inference and check debug information
Deploy the Post Vision App for debugging on a device and run inference.
If you use Console for AITRIOS to run inference, see [README for evaluate](../../../../5_evaluate/README.md).

Check the obtained inference result. The following is an example of inference results that are retrieved using Console for AITRIOS.

```json
{
  "DeviceID": "xxxxx",
  "ModelID": "xxxxx",
  "Image": true,
  "Inferences": [
    {
      "T": "xxxxx",
      "O": "AQAAAA=="
    }
  ],
  "id": "xxxxx",
  "_rid": "xxxxx",
  "_self": "xxxxx",
  "_etag": "xxxxx",
  "_attachments": "xxxxx",
  "_ts": 0
}
```

The debug information is stored in **`"O"`** in **`"Inference"`** in the above json. In the sample code, you can get **`"AQAAAA=="`** as debug information if the PPLParameter is invalid.

The obtained debug information is Base64 encoded and needs to be decoded.
The result of decoding **`"AQAAAA=="`** is "1".

> **NOTE**
> 
> For Python, you can decode the encoded debug information as follows :
> 
> ```python
> import base64
> 
> debug_info = "AQAAAA=="
> int.from_bytes(base64.b64decode(debug_info), 'little')
> ```

The sample code uses **`EPPL_RESULT_CODE`** which is the return value  of **`PPL_ParseParam`** for debug information.

**`EPPL_RESULT_CODE`** is defined in [ppl_public.h](../../ppl_sdk/imx_app/include/ppl_public.h) as follows :

```cpp
typedef enum {
    E_PPL_OK,
    E_PPL_INVALID_PARAM,
    E_PPL_E_MEMORY_ERROR,
    E_PPL_INVALID_STATE,
    E_PPL_OTHER
} EPPL_RESULT_CODE;
```

The obtained debug information "1" corresponds to **`E_PPL_INVALID_PARAM`**, which means that PPLParameter is invalid.
