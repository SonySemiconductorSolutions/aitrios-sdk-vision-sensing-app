# Tutorials - 3. Prepare model

Vision and Sensing Application SDK doesn't provide AI model training and retraining.
You can create AI model by following two ways.

## Train/retrain AI model using Console for AITRIOS

See [Console Manual](https://developer.aitrios.sony-semicon.com/development-guides/documents/manuals/) for details.

- Create model
- Train model
- Labeling & Training

## Train/retrain AI model using your own environment

If you want to create your custom AI model without Console for AITRIOS,

it's recommended the AI model is based on Keras MobileNet image classification (for example, MobileNetV2 based transfer learning AI model), 

because [quantize model](../4_quantize_model/README.md) process supports only Keras MobileNet based image classification model.
