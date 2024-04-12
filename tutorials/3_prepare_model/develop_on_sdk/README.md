# Develop AI model on "**Edge Application SDK**"

"**Edge Application SDK**" provides following functions to prepare AI model:

## 1. Transfer learning
This part describes how to train an AI model. <br>

See [README](./1_train_model/README.md) for details.

## 2. Model quantization

This part describes how to quantize an AI model and convert it into a format that can be imported to "**Console for AITRIOS**". <br>
Supported AI models are based on ["**Model Compression Toolkit (MCT)**"'s features](https://github.com/sony/model_optimization/tree/v1.8.0#supported-features).<br>
See [README](./2_quantize_model/README.md) for details.

## 3. Import model to "**Console for AITRIOS**"

This part describes how to import model to "**Console for AITRIOS**". <br>

See [README](./3_import_to_console/README.md) for details.

> **NOTE**
>
> Azure Blob Storage is required to import an AI model to "**Console for AITRIOS**" using the notebook provided by the SDK.<br>
> If you want to import an AI model from a local environment, use "**Console UI**". See ["**Console User Manual**"](https://developer.aitrios.sony-semicon.com/en/documents/console-user-manual) for details.

## 4. Deploy model to Edge AI Device

This part describes how to deploy model to Edge AI Device. 

See [README](./4_deploy_to_device/README.md) for details.

## More functions
### Delete AI model from "**Console for AITRIOS**"
See [README](./delete_model_on_console/README.md) to get started.

## References

### Image Classification AI model trained in the SDK

The specification depends on a Base AI model used in transfer learning.

If the **`source_keras_model`** in **`./1_train_model/image_classification/configuration.json`** is blank, Keras pre-installed MobileNetV2 model is used. The specification of the AI model is following.

#### Input Tensor

Input data format to AI model is following.

| Category | Description |
| --- | ----------- |
| Color space | RGB |
| Data type | Uint8, 3 channels |
| Shape | 224x224x3 |

#### Output Tensor

Output data format from AI model is following.

| Category | Description |
| --- | ----------- |
| Shape | 5-dim flat vector (If you train 5 classes)<br>The number of dimensions depends on that of trained classes. |
| Values | confidence scores (float in the range [0.0, 1.0])
