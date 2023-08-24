# Vision and Sensing Application Sample for Semantic Segmentation

> **NOTE**
>
> We do not release the AI model corresponding to the "**Vision and Sensing Application**" sample for Semantic Segmentation, so the sample cannot be run on Edge AI Devices.
If you want the sample to run, use [Wasm debugging](../../../../../README_wasmdebug.md).

"**Vision and Sensing Application**" sample for Semantic Segmentation is post-processing of the AI model output to set the class id for each pixel in the image.

The following tables show the output data format of semantic segmentation models supported by the sample.

> **NOTE**
>
> Following input tensor width and height values "125 x 125" depend on the AI model used.

| Category | Description |
| --- | ----------- |
| Shape | 15625(125 x 125)-dim flat vector |
| Values | class indexes |

The array details are as follows:

| Index | Corresponding image coordinates |
| --- | ----------- |
| Idx=0, ..., 124 | x0y0, x0y1, ..., x0y124 |
| Idx=125, ..., 249 | x1y0, x1y1, ..., x1y124 |
| ... | |
| Idx=15501, ..., 15625 | x124y0, x124y1, ..., x124y124 |

## Initialize process
The PPL Parameter is used to process the "**Vision and Sensing Application**" and is passed by **`ConfigurationCallback`** to the "**Vision and Sensing Application**".

This function parses this value and sets it as a variable. Since PPL Parameter is in JSON format, it is parsed using [parson](../../../../third_party/parson/).

The following parameters are used in the sample "**Vision and Sensing Application**" :

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
  > See [**`PPL_ID_VERSION`**](../../../post_process/semanticsegmentation/include/analyzer_semanticsegmentation.h) for sample code values.
  > You can also omit the **`header`** parameter.
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
Even if the PPL Parameter is not specified in the "**Console for AITRIOS**", it is possible to operate by setting an initial value in [the program](../../../post_process/semanticsegmentation/include/analyzer_semanticsegmentation.h). The sample sets the following values :

```json
{
    "header" :
    {
        "id" : "00",
        "version" : "01.01.00"
    },
    "input_width" : 125,
    "input_height" : 125
}
```

## Analyze process
See [/tutorials/4_prepare_application/1_develop/sdk/sample/post_process/semanticsegmentation/](../../../post_process/semanticsegmentation/) for the Analyze process.

**`PPL_SegmentationAnalyze`** provides the function to parse the output of the AI model received in float **`*p_data`** (the output of the IMX500, excluding the format dependent on IMX500), sort it, and output the data in which the class ID is set for each pixel of the **`input_width`** * **`input_height`** image, in the FlatBuffers format.

The sample uses the following for the FlatBuffers schema :

```
namespace SmartCamera;

table SemanticSegmentationData {
  height: ushort;
  width: ushort;
  class_id_map: [ushort];
  num_class_id: ushort;
  score_map: [float]; 
}

table SemanticSegmentationTop {
  perception:SemanticSegmentationData;
}

root_type SemanticSegmentationTop;
```
