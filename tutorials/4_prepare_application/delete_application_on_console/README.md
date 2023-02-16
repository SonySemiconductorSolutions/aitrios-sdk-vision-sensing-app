# Delete Post Vision App from Console for AITRIOS
The SDK provides a Jupyter Notebook for deleting Post Vision App from Console for AITRIOS. <br>
By running the [delete_application_on_console.ipynb](./delete_application_on_console.ipynb), you can delete the Post Vision App from the console.

## Get started
### 1. Setup Console Access Library
To import the Post Vision App into the console, setup access library client.

See [README](./../../_common/set_up_console_client/README.md) to get started.

### 2. Get Post Vision App list
To delete the Post Vision App from the console, get the Post Vision App list and check the app name and version number.

See [README](./../get_application_list/README.md) to get started.

### 3. Create setting file
Place setting file (**`./configuration.json`** file) for deleting.
- configuration.json
    ```json
	{
		"app_name": "",
		"version_number": ""
	}
    ```
> **NOTE**<br>
> See [4. Edit settings](#4-edit-settings) for details on the parameter.

### 4. Edit settings
Edit the parameters in [configuration.json](./configuration.json).

The parameters required to run this notebook are :
|Setting|Description|Range|Required/Optional|Remarks
|:--|:--|:--|:--|:--|
|**`app_name`**|The name of the Post Vision App you want to delete |String. <br>See NOTE. |Required|Used for Console Access Library API:<br>**`deployment.deployment.Deployment.delete_device_app`**|
|**`version_number`**|The version number of the Post Vision App you want to delete |String. <br>See NOTE. |Required|Used for Console Access Library API:<br>**`deployment.deployment.Deployment.delete_device_app`**|

> **NOTE**<br>
> See [API Reference](https://developer.aitrios.sony-semicon.com/development-guides/reference/api-references/) of Console Access Library for other restrictions.

### 5. Run the notebook
Open [notebook](./delete_application_on_console.ipynb) and run the cells.

You can run all cells at once, or you can run the cells one by one.

If successful, Post Vision App will be removed from the Console for AITRIOS and the following log will be displayed:
```bash
Deleting Post Vision App is completed.
	app_name: xxxxxxxxx
	version_number: xxxxxxxxx
```

