# Prepare Model (Image Classification Transfer Learning with Keras)

This tutorial shows you how to train your custom AI model using transfer learning with Keras.

You can generate **Keras saved model** from Keras base AI model (Image Classification).

## Getting Started

### 1. Place base AI model, images and labels

In the development environment,

1. Place your base AI model (for example, **`./model.h5`** file or **`./model`** folder). Supported model formats are **Keras h5 file** or **Keras SavedModel folder**. If you want to use Keras pre-installed MobileNetV2 model, no need to place the AI model.

2. Place JPEG images for training (for example, **`../../../../_common/dataset/training`** folder). The folder structure format is Image Net v1.

    JPEG image format is supported in "**Edge Application SDK**". Other formats depend on [TensorFlow](https://www.tensorflow.org/api_docs/python/tf/io/decode_jpeg).


3. Place JPEG images for validation after training (for example, **`../../../../_common/dataset/validation`** folder). The folder structure format is ImageNet v1.

    JPEG image format is supported in "**Edge Application SDK**". Other formats depend on [TensorFlow](https://www.tensorflow.org/api_docs/python/tf/io/decode_jpeg).

4. Place label file for training and validation (for example, **`../../../../_common/dataset/labels.json`** file). Supported label file format is a dictionary with key as label name and value as class id.

    For example, **`labels.json`** will be following contents.

    - labels.json
        ```
        {
            "car": 1,
            "bike": 2,
            "human": 3
        }
        ```

### 2. Create setting file
Place setting file (**`./configuration.json`** file) for training. 
- configuration.json
    ```json
    {
        "source_keras_model": "",
        "dataset_training_dir": "../../../../_common/dataset/training",
        "dataset_validation_dir": "../../../../_common/dataset/validation",
        "evaluate_label_file":"../../../../_common/dataset/labels.json",
        "batch_size": 32,
        "input_tensor_size": 224,
        "epochs": 10,
        "output_dir": "./output",
        "evaluate_result_dir": "./evaluate/results"
    }
    ```
> **NOTE**<br>
> See [3. Edit settings](#3-edit-settings) for details on the parameter.

### 3. Edit settings
    
Edit the parameters in [configuration.json](./configuration.json).

|Setting|Description|Range|Required/Optional
|:--|:--|:--|:--|
|**`source_keras_model`**|Base AI model path to Keras h5 file or Keras SavedModel folder|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required (If it is blank, Keras pre-installed MobileNetV2 model is used.)|
|**`dataset_training_dir`**|Directory includes JPEG images for training|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|
|**`dataset_validation_dir`**|Directory includes JPEG images for validation|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|
|**`evaluate_label_file`**|Path to label file for evaluation after training|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|
|**`batch_size`**|batch size for training/validation dataset|1 or more. (typical: 32. power of 2 recommended.)|Required|
|**`input_tensor_size`**|input tensor size|**Depends on your custom AI model.** (typical: 224)|Required|
|**`epochs`**|training epoch count|1 or more. (typical: 10)|Required|
|**`output_dir`**|Directory to output trained AI model|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|
|**`evaluate_result_dir`**|Directory to output validation result file|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|

### 4. Edit the notebook (optional)

Implementation of removing top layer of base AI Model depends on your custom AI model's layer structure.

Edit the implementation of **`remove_top_layer_if_needed`**' in [Notebook](./transfer_learning_image_classification_keras_model.ipynb) if needed.

- (Excerpt) implementation

    ```python
    # If base_model includes top (output) layer, we must remove the top (output) layer. For example:
    # remove_top_layer_if_needed START
    if source_keras_model:
        top_layer_offset = -3
        base_model = tf.keras.Model(base_model.layers[0].input,
                                    base_model.layers[top_layer_offset].output)
    # remove_top_layer_if_needed END
    ```

### 5. Run the notebook

Open [Notebook](./transfer_learning_image_classification_keras_model.ipynb) and run the cells.

If successful, **`saved_model`** will be generaged in **`output_dir`**.
And validation result will be saved in **`evaluate_result_dir`**.

You can run all cells at once, or you can run the cells one by one.

> **NOTE**
> 
> Depending on the amount of dataset images or the parameters in configuration.json, the free memory in the development environment may decrease and jupyter kernel may crash while running notebook. Also, continuous execution of the notebook may cause a crash as well.
> 
> To recover, please restart jupyter kernel and re-run notebook.
> 
> Alternatively, please increase your machine spec (for example, configure Codespaces machine types to 8-core 16GB RAM).
