# Post Vision App sample for object detection

Supporting Object Detection models using SSD such as the following.
- SSDMobileNetV1
- SSDLiteMobilenetV2
- Human Detection (available model on Console for AITRIOS)

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
  > Set as "00" in Post Vision App sample for Object Detection.
  - **`version`**<br>
  Restriction: Not null, no other restriction.
  > **NOTE**
  > 
  > Set to 6 digits (8 chars; XX.XX.XX).
  >
  > Set as "01.00.00" in Post Vision App sample for Object Detection.
- **`dnn_output_detections`**<br>
  Description: The maximum number of Bounding boxes which can be detected by Object Detection AI Model.<br>
  Restriction: Not null, no other restriction.
  > **NOTE**
  > 
  > Type of **`dnn_output_detections`** is cast into uint16_t.
- **`max_detections`**<br>
  Description: Threshold of detections number. The maximum number of detected Bounding boxes you want to get after Post Vision App.<br>
  Restriction: **`0 < max_detections <= dnn_output_detections`**<br>
  If **`max_detections > dnn_output_detections`**, **`max_detections`** is set the value of **`dnn_output_detections`**.
  > **NOTE**
  > 
  > Type of **`max_predictions`** is cast into uint16_t.
- **`threshold`**<br>
  Description: Score threshold.<br>
  Restriction: **`0.0 <= threshold <= 1.0`**<br>
  If **`threshold < 0.0`** or **`threshold > 1.0`**, **`threshold`** is set the value of **`PPL_DEFAULT_THRESHOLD`**.
  > **NOTE**
  >
  > Type of **`threshold`** is cast into float.
- **`input_width`**<br>
  Description: Width of AI model's input tensor.<br>
  Restriction: Not null, no other restriction.
  > **NOTE**
  >
  > Type of **`input_width`** is cast into uint16_t.
- **`input_height`**<br>
  Description: Height of AI model's input tensor.<br>
  Restriction: Not null, no other restriction.
  > **NOTE**
  >
  > Type of **`input_height`** is cast into uint16_t.

Even if the **`PPLParameter`** is not specified in the "Console for AITRIOS", it is possible to operate by setting an initial value in the program.
The sample sets the following values for SSDMobileNetV1 :
```json
{
    "header" :
    {
        "id" : "00",
        "version" : "01.00.00"
    },
    "dnn_output_detections" : 10,
    "max_detections" : 10,
    "threshold" : 0.3,
    "input_width" : 300,
    "input_height" : 300
}
```

You need to set appropriate values for your AI model.

For Human Detection (available model on Console for AITRIOS), set the following values :
```json
{
    "header" :
    {
    "id" : "00",
    "version" : "01.00.00"
    },
    "dnn_output_detections" : 50,
    "max_detections" : 5,
    "threshold" : 0.6,
    "input_width" : 300,
    "input_height" : 300
}
```

If **`PPL_Initialize`** succeeds, it changes the status to **`PPL_IDLE`**. (If no appropriate parameter is given, the **`PPL_Analyze`** fails because the state does not transition.)

### PPL_Analyze

It provides the function to parse the output of the AI model received in **`float *p_data`** (the output of the IMX500, excluding the format dependent on IMX500), sort it, and output the data of the number of **`max_detections`** specified in **`PPLParameter`**, in the Flatbuffer format.

The sample uses the following for the Flatbuffer schema :
```
namespace SmartCamera;

table BoundingBox2d {
  left:int;
  top:int;
  right:int;
  bottom:int;
}

union BoundingBox {
  BoundingBox2d,
}

table GeneralObject {
  class_id:uint;
  bounding_box:BoundingBox;
  score:float;
}

table ObjectDetectionData {
  object_detection_list:[GeneralObject];
}

table ObjectDetectionTop {
  perception:ObjectDetectionData;
}

root_type ObjectDetectionTop;
```
