# Zone Detection

> **NOTE**
>
> To complete all steps, Azure Blob Storage is required when importing AI model to "**Console for AITRIOS**".
This is because "**Console Access Library**" used in the notebook does not support importing AI model from a local environment.
This sample is intended to execute all steps with python scripts.

## Learn the SDK workflow in a day

### Terms

- "**Zone Detection**"

    ["**Zone Detection**"](https://developer.aitrios.sony-semicon.com/development-guides/tutorials/sample-application/) is a sample application of AITRIOS.

- "**Vision and Sensing Application**"

    "**Vision and Sensing Application**" is post-processing application of AI model's Output Tensor running on Edge AI Device.

    Please see [../../tutorials/4_prepare_application/1_develop/README.md](../../tutorials/4_prepare_application/1_develop/README.md) for details.

### Overview

A sample jupyter notebook demonstrates a workflow of "**Vision and Sensing Application SDK**" for "**Zone Detection**".

In "**Vision and Sensing Application**" for "**Zone Detection**", by calculating IoU (Intersection over Union) in Edge AI Device, there is an advantage that the amount of uploading data to the cloud can be reduced.

#### Example of an original image as input and an image with inference results as output

<img alt="example of an original image as input and an image with inference result as output" src="./Images_README/overview_example.png" width="700px" />

### Workflow

1. Prepare dataset<br>
Prepare the dataset pre-installed in the SDK.
2. Prepare AI model<br>
Download a learned model and perform transfer learning using the preceding dataset.
3. Prepare "**Vision and Sensing Application**"<br>
Optimize the "**Vision and Sensing Application**" provided by the SDK by changing parameters for the "**Vision and Sensing Application**".
4. Run "**Vision and Sensing Application**" with mock<br>
Run the "**Vision and Sensing Application**" on the SDK before deploying it to Edge AI Devices.
5. Deploy AI model and "**Vision and Sensing Application**"<br>
Import the AI model and the "**Vision and Sensing Application**" to "**Console for AITRIOS**" and deploy them to Edge AI Devices.
6. Evaluate<br>
Run inference using "**Console for AITRIOS**".<br>
This chapter requires a subject of images.
We recommend car and parking objects like the ones in the image of the dataset preinstalled in the SDK.
There is no problem using a picture instead of actual objects.

### Instruction

Please open [sdk_in_a_day.ipynb](./sdk_in_a_day.ipynb) and run step-by-step (one cell a time).

### References

#### Object Detection AI model trained in the sample jupyter notebook

##### Input Tensor

Input data format to AI model is following.

| Category | Description |
| --- | ----------- |
| Color space | RGB |
| Data type | Uint8, 3 channels |
| Shape | 300x300x3 |

##### Output Tensor

Output data format from AI model is following.

| Category | Description |
| --- | ----------- |
| Shape | 61-dim flat vector<br>The number of maximum detections is set to 10. |
| Values | Idx=0, ..., 9: y_min coordinates (float in the range [0.0, 1.0])<br>Idx=10, ..., 19: x_min coordinates (float in the range [0.0, 1.0])<br>Idx=20, ..., 29: y_max coordinates (float in the range [0.0, 1.0])<br>Idx=30, ..., 39: x_max coordinates (float in the range [0.0, 1.0])<br>Idx=40, ..., 49: class indexes<br>Idx=50, ..., 59: confidence scores (float in the range [0.0, 1.0])<br>Idx=60: number of detections
