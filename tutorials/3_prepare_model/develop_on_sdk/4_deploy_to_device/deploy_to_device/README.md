# Deploy AI model to edge AI devices

This tutorial shows how to deploy AI model to edge AI devices.

## Get started

### 1. Set up "**Console Access Library**"
Set up access library client to deploy AI model.

See [README](../../../../_common/set_up_console_client/README.md) for details.

### 2-a. Get model list (optional)
You need **`model_id`** and **`model_version_number`** of the model in case you create a new configuration for deploying to edge AI devices. 

If you want to get information on an AI model imported to "**Console for AITRIOS**", get the model list and check model ID and version number.

See [README](../../get_model_list/README.md) for details.

### 2-b. Get deploy configuration list (optional)
You need **`config_id`** of a deployment configuration registered in "**Console for AITRIOS**" in case you use an existing deploy configuration. 

If you want to get information on deployment configuration IDs, get the configuration list and check the config ID.

See [README](../../get_deploy_config/README.md) for details.

### 3. Get list of devices and models (optional)
You need **`device_ids`** of the target devices to deploy the model.

You also need **`replace_model_id`** if you need to replace a deployed model because the maximum number of models you can deploy to a device is 4.

If you want to get information on your devices and models, get the list of devices and models and check the device ID and model ID.

See [README](../../get_device_list/README.md) for details.

### 4. Create setting file
Place setting file (**`./configuration.json`** file) for deploying.
- configuration.json
    ```json
    {
        "should_create_deploy_config" : true,
        "config_id" : "",
        "create_config": {
            "comment" : "",
            "model_id": "",
            "model_version_number" : ""
        },
        "device_ids": [""],
        "replace_model_id" : "",
        "comment" : ""
    }
    ```

- configuration.json (example without optional parameters)
    ```json
    {
        "should_create_deploy_config" : true,
        "config_id" : "",
        "device_ids": [""]
    }
    ```

> **NOTE**<br>
> See [5. Edit settings](#5-edit-settings) for details on the parameter.

### 5. Edit settings
Edit the parameters in [configuration.json](./configuration.json).

The parameters required to run this notebook are :

|Setting||Description|Range|Required/Optional|Remarks
|:--|:--|:--|:--|:--|:--|
|**`should_create_deploy_config`**||Whether to register a new deploy configuration.<br>If true, create a new configuration; if false, use an already registered configuration specified by **`config_id`**.|true or false|Required||
|**`config_id`**||The ID of the deploy configuration you want to use for deployment.|String.<br>See NOTE.|Required|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.create_deploy_configuration`**<br>**`deployment.deployment.Deployment.deploy_by_configuration`**|
|**`create_config`**|**`comment`**|Description of the configuration.|String.See NOTE.|Optional|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.create_deploy_configuration`**|
||**`model_id`**|The ID of the model you want to deploy.|String.<br>See NOTE.|Optional<br> Required if **`should_create_deploy_config`** is true.|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.create_deploy_configuration`**|
||**`model_version_number`**|The version of the model you want to deploy.|String.<br>See NOTE.|Optional|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.create_deploy_configuration`**|
|**`device_ids`**||List of device ids on which you want to deploy your model.|List of string.|Required|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.deploy_by_configuration`**|
|**`replace_model_id`**||The ID of the model you want to replace.|String.<br>See NOTE.|Optional|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.deploy_by_configuration`**|
|**`comment`**||Description of the deployment.|String.<br>See NOTE.|Optional|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.deploy_by_configuration`**|

> **NOTE**
>
> See [API Reference](https://developer.aitrios.sony-semicon.com/en/edge-ai-sensing/guides/) of "**Console Access Library**" for other restrictions.

### 6. Run the notebook
Open the [notebook](./deploy_to_device.ipynb) and run the cells.

You can run all cells at once, or you can run the cells one by one.

If successful, AI model will be deployed to Edge AI devices and the following log will be displayed:

```
Start to deploy the model.
```

Deploying model will take some time, so run the "Check deployment status" cell to check the result of deployment.

When the "Check deployment status" cell is executed, the deployment status of each device is displayed as follows:

|**`device_id`**|**`deploy_id`**|**`config_id`**|**`deploy_status`**|**`model_status`**|**`update_date`**|
|:--|:--|:--|:--|:--|:--|
|device_id_xxxx|xxxx|config_xxxx|Deploying|Deploying|20xx-01-01T00:00:00.000000+00:00|
|device_id_yyyy|yyyy|config_yyyy|Success|Success|20xx-01-01T00:00:00.000000+00:00|

The deploy status is "Deploying", "Success", "Failed", or "Canceled".

The model status is "Waiting for execution", "Deploying", "Success", or "Failed".
