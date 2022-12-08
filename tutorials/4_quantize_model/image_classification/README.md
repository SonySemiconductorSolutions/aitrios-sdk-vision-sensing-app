# Quantize Model (Image Classification Keras Model to TFLite Model)

This tutorial shows you how to quantize your custom AI model using [Model Compression Toolkit (MCT)](https://github.com/sony/model_optimization).

You can generate **quantized TFLite model** from your custom AI model (Image Classification Keras Model).

## Getting Started

### 1. Place custom AI model, images and ground truth

In the development environment,

1. Place your custom AI model (for example, **`./model.h5`** file or **`./model`** folder). Supported model formats are **Keras h5 file** or **Keras Saved Model folder**.

2. Place jpeg images for calibration of quantization (for example, **`./images`** folder). It is recommended to place the images used in learning or validating your custom AI model. A typical number of images is 300. Accuracy of the quantized AI model depends on the images you placed.

3. Place jpeg images for evaluation after quantization (for example, **`./evaluate/images`** folder).

4. Place grund truth file for evaluation after quantization (for example, **`./evaluate/ground_truth.txt`** file). Supported ground truth file format is that class ids are listed per row in the same order as the image filename.

    For example, when id, label and image file are following, **`ground_truth.txt`** will be following contents.

    - id and label
        - 0 : car
        - 1 : bike
        - 2 : human

    - image file
        - bike1.JPG
        - bike2.JPG
        - car1.JPG
        - human1.JPG
        - human2.JPG

    - ground_truth.txt
        ```
        1
        1
        0
        2
        2
        ```

### 2. Edit settings
    
Set the parameters in [configuration.json](./configuration.json).

|Setting|Description|Range|Required/Optional
|:--|:--|:--|:--|
|**`source_keras_model`**|Path to Keras h5 file or Keras Saved Model folder|Absolute path or relative path from configuration.json/Notebook(*.ipynb)|Required|
|**`dataset_image_dir`**|Directory includes jpeg images for calibration of quantization|Absolute path or relative path from configuration.json/Notebook(*.ipynb)|Required|
|**`batch_size`**|a number of images for subdividing calibration for quantization|1 - total number of images for calibration. (typical: 50)|Required|
|**`input_tensor_size`**|input tensor size|**Depends on your custom AI model.** (typical: 224)|Required|
|**`iteration_count`**|quantization iteration count|1 or more. (typical: 10)|Required|
|**`output_dir`**|Directory to output quantized AI model|Absolute path or relative path from configuration.json/Notebook(*.ipynb)|Required|
|**`evaluate_image_dir`**|Directory includes jpeg images for evaluation after quantization|Absolute path or relative path from configuration.json/Notebook(*.ipynb)|Required|
|**`evaluate_image_extension`**|extension of jpeg images for evaluation after quantization|string of extension (typical: JPEG)|Required|
|**`evaluate_ground_truth_file`**|Path to ground truth file for evaluation after quantization|Absolute path or relative path from configuration.json/Notebook(*.ipynb)|Required|
|**`evaluate_result_dir`**|Directory to output evaluation result file|Absolute path or relative path from configuration.json/Notebook(*.ipynb)|Required|

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

### 5. (Optional) Import the model_quantized.tflite to Console for AITRIOS

If you want to import the **`model_quantized.tflite`** to Console for AITRIOS,

see [Console Manual](https://developer.aitrios.sony-semicon.com/development-guides/documents/manuals/) for details.

- Create model

    > **NOTE**
    > 
    > When importing the model on Console for AITRIOS, clear "CustomVision" check and clear "Converted Model" check.
    >
    > How to deploy models to device is described in [README for deploy](../../6_deploy/README.md).
    >
