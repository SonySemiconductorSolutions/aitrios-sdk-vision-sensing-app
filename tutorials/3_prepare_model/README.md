# Tutorials - 3. prepare model

This tutorial shows you how to prepare AI model that can run on Edge AI Devices.


## Prepare AI model on "**Edge Application SDK**"

You can train, quantize and import AI model to "**Console for AITRIOS**" and deploy it to Edge AI Devices using the SDK.

See [README](./develop_on_sdk/README.md) to get started.

## Prepare AI model on "**Console for AITRIOS**"

Alternatively, you can train and deploy AI model using "**Console for AITRIOS**".

See [README](./develop_on_console/README.md) to get started.

## Prepare AI model on your own environment

You can use your own environment to train your custom AI model. 
The custom AI model can be quantized, imported to "**Console for AITRIOS**", and deployed to Edge AI Device using SDK.

In that case, it is recommended the AI model is based on Keras MobileNet image classification (for example, MobileNetV2 based transfer learning AI model), because [quantize model](./develop_on_sdk/2_quantize_model/README.md) process supports only Keras MobileNet based image classification model.

