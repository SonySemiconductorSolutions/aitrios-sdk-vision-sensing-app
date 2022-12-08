# Vision and Sensing Application SDK Tutorial
You can start Vision and Sensing Application SDK workflow using the following tutorial.

## 1. Initialize
This part describes how to start using Console for AITRIOS.

See [README](./1_initialize/README.md) for details.

## 2. Prepare dataset
This part describes how to prepare images and datasets for training and evaluation of your custom AI models.

It contains the following sub-parts:
- download images
  - You can download COCO images using a Jupyter notebook.
- annotate images
  - You can annotate images using VoTT.
  - Also, you can convert dataset format from VoTT to COCO using a Jupyter notebook.

The COCO images may also be useful as training images if you want to train or retrain AI model on Console for AITRIOS (and if your training AI model's label is also included in COCO dataset's categories).

See [README](./2_prepare_dataset/README.md) for details.

## 3. Prepare model
This part describes how to train AI models. <br>

Dev Container does not provide a training environment for AI models.<br>
Train using Console for AITRIOS or prepare your own training environment.<br>
See [README](./3_prepare_model/README.md) for details.

## 4. Quantize model
This part describes how to quantize an AI model and convert it into a format that can be uploaded to Console for AITRIOS. <br>
Supported AI models are based on [Model Compression Toolkit (MCT)'s features](https://github.com/sony/model_optimization/tree/v1.3.0#supported-features).<br>

See [README](./4_quantize_model/README.md) for details.

## 5. Post process
This part describes how to develop and build a post-process application.<br>

See [README](./5_post_process/README.md) for details.

## 6. Deploy
This part describes how to deploy AI models and applications to devices using Console for AITRIOS.<br>

See [README](./6_deploy/README.md) for details.

## 7. Evaluate
This part describes how to evaluate the output results of AI models and applications on Console for AITRIOS. <br>

See [README](./7_evaluate/README.md) for details.
