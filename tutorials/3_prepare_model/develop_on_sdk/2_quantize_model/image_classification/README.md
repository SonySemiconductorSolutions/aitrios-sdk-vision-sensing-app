# Quantize Model (Image Classification Keras Model to TFLite Model)

This tutorial shows you how to quantize your custom AI model using ["**Model Compression Toolkit (MCT)**"](https://github.com/sony/model_optimization).

You can generate **quantized TFLite model** from your custom AI model (Image Classification Keras Model).

## Getting Started

### 1. Place custom AI model, images and ground truth

In the development environment,

1. Place your custom AI model (for example, **`./model.h5`** file or **`./model`** folder). Supported model formats are **Keras h5 file** or **Keras SavedModel folder**.

2. Place JPEG images for calibration of quantization (for example, **`../../../../_common/dataset/training`** folder). It is recommended to place the images used in learning or validating your custom AI model. A typical number of images is 300. Accuracy of the quantized AI model depends on the images you placed.

    JPEG image format is supported in "**Edge Application SDK**". Other formats depend on [TensorFlow](https://www.tensorflow.org/api_docs/python/tf/io/decode_jpeg) and ["**MCT**"](https://sony.github.io/model_optimization/docs/api/experimental_api_docs/classes/FolderImageLoader.html?highlight=folderimageloader#default-file-types-to-scan).

3. Place JPEG images for evaluation after quantization (for example, **`../../../../_common/dataset/validation`** folder).

    JPEG image format is supported in "**Edge Application SDK**". Other formats depend on [TensorFlow](https://www.tensorflow.org/api_docs/python/tf/io/decode_jpeg) and ["**MCT**"](https://sony.github.io/model_optimization/docs/api/experimental_api_docs/classes/FolderImageLoader.html?highlight=folderimageloader#default-file-types-to-scan).

4. Place label file for evaluation after quantization (for example, **`../../../../_common/dataset/labels.json`** file). Supported label file format is a dictionary with key as label name and value as class id.

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
Place setting file (**`./configuration.json`** file) for quantization. 
- configuration.json
    ```json
    {
        "source_keras_model": "../../1_train_model/image_classification/output/saved_model",
        "dataset_image_dir":"../../../../_common/dataset/training",
        "batch_size": 50,
        "input_tensor_size": 224,
        "iteration_count": 10,
        "output_dir":"./output",
        "evaluate_image_dir":"../../../../_common/dataset/validation",
        "evaluate_image_extension": "jpg",
        "evaluate_label_file":"../../../../_common/dataset/labels.json",
        "evaluate_result_dir":"./evaluate/results"
    }
    ```
> **NOTE**<br>
> See [3. Edit settings](#3-edit-settings) for details on the parameter.

### 3. Edit settings
    
Edit the parameters in [configuration.json](./configuration.json).

|Setting|Description|Range|Required/Optional
|:--|:--|:--|:--|
|**`source_keras_model`**|Path to Keras h5 file or Keras SavedModel folder|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|
|**`dataset_image_dir`**|Directory includes JPEG images for calibration of quantization|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|
|**`batch_size`**|a number of images for subdividing calibration for quantization|1 - total number of images for calibration. (typical: 50)|Required|
|**`input_tensor_size`**|input tensor size|**Depends on your custom AI model.** 1 or more integer. (typical: 224)|Required|
|**`iteration_count`**|quantization iteration count|1 or more integer. (typical: 10)|Required|
|**`output_dir`**|Directory to output quantized AI model|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|
|**`evaluate_image_dir`**|Directory includes JPEG images for evaluation after quantization|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|
|**`evaluate_image_extension`**|extension of JPEG images for evaluation after quantization|string of extension (typical: jpg)|Required|
|**`evaluate_label_file`**|Path to label file for evaluation after quantization|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|
|**`evaluate_result_dir`**|Directory to output evaluation result file|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|

### 3. Edit the notebook (optional)

Implementation of preprocessing images (The type is np.uint8 RGB.) on calibration of quantization depends on your custom AI model's preprocessing images on learning.

Edit the implementation of **`FolderImageLoader`**'s argument of **`preprocessing`** (The type is List[Callable].) in [Notebook](./quantize_image_classification_keras_model_with_mct.ipynb) if needed.

- (Excerpt) implementation

    ```python
    def resize(x):
        ...

    def normalization(x):
        ...

    image_data_loader = FolderImageLoader(dataset_image_dir,
                                          preprocessing=[resize, normalization],
                                          batch_size=batch_size)
    ```

### 4. Run the notebook

Open [Notebook](./quantize_image_classification_keras_model_with_mct.ipynb) and run the cells.

If successful, **`model_quantized.tflite`** will be generaged in **`output_dir`**.
And evaluation result will be saved in **`evaluate_result_dir`**.

You can run all cells at once, or you can run the cells one by one.

> **NOTE**
> 
> Depending on the amount of dataset images or the quantization parameters in configuration.json, the free memory in the development environment may decrease and jupyter kernel may crash while running notebook. Also, continuous execution of the notebook may cause a crash as well.
> 
> To recover, please restart jupyter kernel and re-run notebook.
> 
> Alternatively, please increase your machine spec (for example, configure Codespaces machine types to 8-core 16GB RAM).
