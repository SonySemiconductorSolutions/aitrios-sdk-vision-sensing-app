# Edge Application Sample for Object Detection
"**Edge Application**" sample for Object Detection is post-processing of the AI model output to limit the output by setting the maximum number of detections and the threshold.

Supporting Object Detection models using SSD such as the following.
- Custom Vision (available model on "**Console for AITRIOS**")

## Initialize process
The PPL Parameter is used to process the "**Edge Application**" and is passed by **`ConfigurationCallback`** to the "**Edge Application**".

This function parses this value and sets it as a variable. Since PPL Parameter is in JSON format, it is parsed using [parson](../../../../third_party/parson/).

The following parameters are used in the sample "**Edge Application**" :

- **`header`**
  - **`id`**<br>
  Description: id.
  > NOTE
  >
  > Set to 2 digits.
  - **`version`**<br>
  Description: version.
  > NOTE
  >
  > Set to 6 digits (8 chars; XX.XX.XX).

  > NOTE
  >
  > If you use the sample, the **`id`** and **`version`** values of the **`header`** must match the values hard-coded in the sample code.
  > See [**`PPL_ID_VERSION`**](../../../post_process/objectdetection/include/analyzer_objectdetection.h) for sample code values.
  > You can also omit the **`header`** parameter.
- **`dnn_output_detections`**<br>
  Description: The maximum number of Bounding boxes which can be detected by Object Detection AI Model.
  > NOTE
  >
  > The value of the parameter depends on the AI model.<br>
  > For the Custom Vision model, set the value to 64.

- **`max_detections`**<br>
  Description: Threshold of detections number. The maximum number of detected Bounding boxes you want to get after "**Edge Application**".
  > NOTE
  >
  > For the Custom Vision model, the value can be changed between 0 and 64. If an invalid value is entered, the default value is applied.
- **`threshold`**<br>
  Description: Score threshold.
  > NOTE
  >
  > The value can be changed between 0 and 1. If an invalid value is entered, the default value is applied.
- **`input_width`**<br>
  Description: Width of AI model's input tensor.
  > NOTE
  >
  > The value of the parameter depends on the AI model.
- **`input_height`**<br>
  Description: Height of AI model's input tensor.
  > NOTE
  >
  > The value of the parameter depends on the AI model.

### PPL Parameter
Even if the PPL Parameter is not specified in the "**Console for AITRIOS**", it is possible to operate by setting an initial value in [the program](../../../post_process/objectdetection/include/analyzer_objectdetection.h). The sample sets the following values for Custom Vision model :

```json
{
    "header" :
    {
        "id" : "00",
        "version" : "01.01.00"
    },
    "dnn_output_detections" : 64,
    "max_detections" : 5,
    "threshold" : 0.3,
    "input_width" : 320,
    "input_height" : 320
}
```

## Analyze process
See [/tutorials/4_prepare_application/1_develop/sdk/sample/post_process/objectdetection/](../../../post_process/objectdetection/) for the Analyze process.

**`PPL_ObjectDetectionSsdAnalyze`** provides the function to parse the output of the AI model received in float **`*p_data`** (the output of the IMX500, excluding the format dependent on IMX500), sort it, and output the data of the number of **`max_detections`** specified in PPL Parameter, in the FlatBuffers format.

The sample uses the following for the FlatBuffers schema :

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
