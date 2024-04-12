# Delete "**Edge Application**" from "**Console for AITRIOS**"
The SDK provides a Jupyter Notebook for deleting "**Edge Application**" from "**Console for AITRIOS**". <br>
By running the [delete_application_on_console.ipynb](./delete_application_on_console.ipynb), you can delete the "**Edge Application**" from the "**Console for AITRIOS**".

## Get started
### 1. Setup "**Console Access Library**"
To delete the "**Edge Application**" from the "**Console for AITRIOS**", setup access library client.

See [README](./../../_common/set_up_console_client/README.md) to get started.

### 2. Get "**Edge Application**" list
To delete the "**Edge Application**" from the "**Console for AITRIOS**", get the list of "**Edge Applications**" and check the **`app_name`** and **`version_number`**.

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
|**`app_name`**|The name of the "**Edge Application**" you want to delete |String. <br>See NOTE. |Required|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.delete_device_app`**|
|**`version_number`**|The version number of the "**Edge Application**" you want to delete |String. <br>See NOTE. |Required|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.delete_device_app`**|

> **NOTE**<br>
> See [API Reference](https://developer.aitrios.sony-semicon.com/development-guides/reference/api-references/) of "**Console Access Library**" for other restrictions.

### 5. Run the notebook
Open [notebook](./delete_application_on_console.ipynb) and run the cells.

You can run all cells at once, or you can run the cells one by one.

If successful, "**Edge Application**" will be removed from the "**Console for AITRIOS**" and the following log will be displayed:
```bash
Deleting Vision and Sensing Application is completed.
	app_name: xxxxxxxxx
	version_number: xxxxxxxxx
```

