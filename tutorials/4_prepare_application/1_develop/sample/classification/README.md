# Post Vision App sample for image classification

Supporting Image Classification models such as the following.
- MobileNetV1
- MNasNet

## Implementation
### PPL_Initialize
In **`PPL_Initialize`**, the **`PPLParameter`** specified for inference execution on "Console for AITRIOS" is passed as an argument as **`const char *p_param`**.
This function parses this value and sets it as a variable. Since **`p_param`** is in JSON format, it is parsed using [parson](../../ppl_sdk/lib/parson/).

The following parameters are used in the sample Post Vision App :

- **`header`**
  - **`id`**<br>
  Restriction: Not null, no other restriction.
  > **NOTE**
  >
  > Set to 2 digits.
  >
  > Set as "00" in Post Vision App sample for Image Classification.
  - **`version`**<br>
  Restriction: Not null, no other restriction.
  > **NOTE**
  > 
  > Set to 6 digits (8 chars; XX.XX.XX).
  >
  > Set as "01.00.00" in Post Vision App sample for Image Classification.
- **`dnn_output_classes`**<br>
  Description: The total number of classes in AI model output.<br>
  Restriction: Not null, no other restriction.
  > **NOTE**
  > 
  > Type of **`dnn_output_classes`** is cast into uint16_t.
- **`max_predictions`**<br>
  Description: Threshold of predictions number. The maximum number of predictions you want to get after Post Vision App.<br>
  Restriction: **`0 < max_predictions <= dnn_output_classes`**<br>
  If **`max_predictions > dnn_output_classes`**, **`max_predictions`** is set the value of **`dnn_output_detections`**.
  > **NOTE**
  > 
  > Type of **`max_predictions`** is cast into uint16_t.

Even if the **`PPLParameter`** is not specified in the "Console for AITRIOS", it is possible to operate by setting an initial value in the program.
The sample sets the following values for MobileNetV1 :
```json
{
    "header": {
        "id": "00",
        "version": "01.00.00"
    },
    "dnn_output_classes": 1001,
    "max_predictions": 5
}
```

You need to set appropriate values for your AI model.

If **`PPL_Initialize`** succeeds, it changes the status to **`PPL_IDLE`**. (If no appropriate parameter is given, the **`PPL_Analyze`** fails because the state does not transition.)

### PPL_Analyze

It provides the function to parse the output of the AI model received in **`float *p_data`** (the output of the IMX500, excluding the format dependent on IMX500), sort it, and output the data of the number of **`max_predictions`** specified in **`PPLParameter`**, in the Flatbuffer format.

The sample uses the following for the Flatbuffer schema :
```
namespace SmartCamera;

table GeneralClassification {
  class_id:uint;
  score:float;
}

table ClassificationData {
  classification_list:[GeneralClassification];
}

table ClassificationTop {
  perception:ClassificationData;
}

root_type ClassificationTop;
```

For MobileNetV1, 1001 float values are passed as the argument **`p_data`**, so they are sorted and stored in Flatbuffer by the number of **`max_predictions`** set in **`PPLParameter`**.
