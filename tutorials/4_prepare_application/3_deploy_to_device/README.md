# Deploy application from "**Console for AITRIOS**" to Edge AI Device

The SDK provides a Jupyter Notebook to deploy the "**Edge Application**" to Edge AI Device.

By running [deploy_to_device.ipynb](./deploy_to_device.ipynb), you can deploy the "**Edge Application**" to Edge AI Device.

## Get started

### 1. Setup "**Console Access Library**"
To deploy the "**Edge Application**" to devices, setup access library client.

See [README](./../../_common/set_up_console_client/README.md) for details.

### 2. Get the list of "**Edge Applications**" from "**Console for AITRIOS**" (optional)

To deploy the "**Edge Application**" to devices, you need **`app_name`** and **`version_number`**.

You can obtain **`app_name`** and **`version_number`** of the "**Edge Application**" imported on the "**Console**" by using Notebook to get device list.

See [README](./../../4_prepare_application/get_application_list/README.md) for details. 

### 3. Get device list from "**Console for AITRIOS**" (optional)

To deploy the "**Edge Application**" to devices, you need **`device_ids`** of the target devices.

You can obtain **`device_ids`** by using Notebook to get device list.

See [README](./../../4_prepare_application/get_device_list/README.md) for details.

### 4. Create setting file
Place setting file (**`./configuration.json`** file) for deploying.
- configuration.json
    ```json
	{
		"app_name": "",
		"version_number": "",
		"device_ids": [""],
		"comment": ""
	}
    ```

- configuration.json (example without optional parameters)
    ```json
	{
		"app_name": "",
		"version_number": "",
		"device_ids": [""]
	}
    ```

> **NOTE**<br>
> See [5. Edit settings to deploy application](#5-edit-settings-to-deploy-the-edge-application) for details on the parameter.

### 5. Edit settings to deploy the "**Edge Application**"

Edit the parameters in [configuration.json](./configuration.json).

The parameters required to run this notebook are:

|Setting|Description|Range|Required/Optional|Remarks
|:--|:--|:--|:--|:--|
|**`app_name`**|The name of the "**Edge Application**" you want to deploy.|String. See NOTE.|Required|Used for "**Console Access Library**" API: <br> **`deployment.deployment.Deployment.deploy_device_app`** <br> **`deployment.deployment.Deployment.get_device_app_deploys`**|
|**`version_number`**|The version number of the "**Edge Application**" you want deploy.|String. See NOTE.|Required|Used for "**Console Access Library**" API: <br> **`deployment.deployment.Deployment.deploy_device_app`** <br> **`deployment.deployment.Deployment.get_device_app_deploys`**|
|**`device_ids`**|List of device ids on which you want to deploy your "**Edge Application**".|List of strings.|Required|Used for "**Console Access Library**" API: <br> **`deployment.deployment.Deployment.deploy_device_app`**|
|**`comment`**|Description of the deployment.|String. See NOTE.|Optional|Used for "**Console Access Library**" API: <br> **`deployment.deployment.Deployment.deploy_device_app`**|

> **NOTE**<br>
> See [API Reference](https://developer.aitrios.sony-semicon.com/development-guides/reference/api-references/) of "**Console Access Library**" for other restrictions.

### 6. Run the notebook to deploy the "**Edge Application**"

Open [deploy_to_device.ipynb](./deploy_to_device.ipynb) and run the cells.

You can run all cells at once, or you can run the cells one by one.

If successful, the "**Edge Application**" will be deployed to Edge AI devices and the following log will be displayed:

```bash
Start to deploy application. 
	app_name: xxxx
	version_number: xxxx
	device_ids: xxxx
```

Deploying "**Edge Applications**" will take some time, so run the "Check deployment status" cell to check the result of deployment.

When the "Check deployment status" cell is executed, the deployment status of each device is displayed as follows:

|**`device_id`**| **`status`** |
|:--|:--|
|device_id_xxxx|Success|
|device_id_yyyy|Success|

The status is "Deploying", "Success", "Fail", or "Cancel".
