# Edge Application Sample for Image Classification
"**Edge Application**" sample for Image Classification is post-processing of the AI model output to limit the output by setting the maximum number of predictions.

Supporting Image Classification models such as the following.
- Image Classification AI model transfer learned and quantized in the SDK

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
  > See [**`PPL_ID_VERSION`**](../../../post_process/classification/include/analyzer_classification.h) for sample code values.
  > You can also omit the **`header`** parameter.
- **`dnn_output_classes`**<br>
  Description: The total number of classes in AI model output.
  > NOTE
  >
  > The value of the parameter depends on the AI model.
- **`max_predictions`**<br>
  Description: Threshold of predictions number. The maximum number of predictions you want to get after "**Edge Application**".
  > NOTE
  >
  > For Image Classification AI model transfer learned and quantized in the SDK, it can be changed from 0 to the number of classes set in transfer learning in the SDK. If an invalid value is entered, the default value is applied.

### PPL Parameter
Even if the PPL Parameter is not specified in the "**Console for AITRIOS**", it is possible to operate by setting an initial value in [the program](../../../post_process/classification/include/analyzer_classification.h). The sample sets the following values :

```json
{
    "header": {
        "id": "00",
        "version": "01.01.00"
    },
    "dnn_output_classes": 18,
    "max_predictions": 5
}
```

> **NOTE**
>
> The preceding PPL Parameter is used when the number of classes of AI model you use is set to 18.

## Analyze process
See [/tutorials/4_prepare_application/1_develop/sdk/sample/post_process/classification/](../../../post_process/classification/) for the Analyze process.

**`PPL_ClassificationAnalyze`** provides the function to parse the output of the AI model received in float **`*p_data`** (the output of the IMX500, excluding the format dependent on IMX500), sort it, and output the data of the number of **`max_predictions`** specified in PPL Parameter, in the FlatBuffers format.

The sample uses the following for the FlatBuffers schema :

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

If you set the number of classes to 18 in the SDK's transfer learning, 18 uint and float values are passed as the argument **`p_data`**, so they are sorted and stored in FlatBuffers by the number of **`max_predictions`** set in PPL Parameter.
