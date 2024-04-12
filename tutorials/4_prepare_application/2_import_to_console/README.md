# Import "**Edge Application**" to "**Console for AITRIOS**"
The SDK provides a Jupyter Notebook to import "**Edge Application**" to "**Console for AITRIOS**". <br>
By running the [import_to_console.ipynb](./import_to_console.ipynb), you can import the "**Edge Application**" into the console and you are ready to deploy it to the device.

## Get started
### 1. Setup "**Console Access Library**"
To import the "**Edge Application**" into the "**Console for AITRIOS**", setup access library client.

See [README](./../../_common/set_up_console_client/README.md) to get started.

### 2. Get the list of "**Edge Applications**" (optional)
If you want to upgrade an "**Edge Application**", get the list of "**Edge Application**" and check the **`app_name`** and **`version_number`**.

See [README](./../get_application_list/README.md) to get started.

### 3. Create setting file
Place setting file (**`./configuration.json`** file) for importing.
- configuration.json
    ```json
	{
		"app_name": "",
		"version_number": "",
		"ppl_file": "./sample/vision_app_classification.wasm",
		"comment": ""
	}
    ```

- configuration.json (example without optional parameters)
    ```json
	{
		"app_name": "",
		"version_number": "",
		"ppl_file": "./sample/vision_app_classification.wasm"
	}
    ```

> **NOTE**<br>
> See [4. Edit settings](#4-edit-settings) for details on the parameter.

### 4. Edit settings
Edit the parameters in [configuration.json](./configuration.json).

The parameters required to run this notebook are :
|Setting|Description|Range|Required/Optional|Remarks
|:--|:--|:--|:--|:--|
|**`app_name`**|The name of the "**Edge Application**" you want to import|String. <br>See NOTE. |Required|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.import_device_app`**|
|**`version_number`**|The version number of the "**Edge Application**" you want to import |String. <br>See NOTE. |Required|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.import_device_app`** |
|**`ppl_file`**|The path to the "**Edge Application**" file for importing |Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required||
|**`comment`**|The description of the "**Edge Application**" |String. <br>See NOTE. |Optional|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.import_device_app`** |

> **NOTE**<br>
> See [API Reference](https://developer.aitrios.sony-semicon.com/development-guides/reference/api-references/) of "**Console Access Library**" for other restrictions.

### 5. Run the notebook
Open [notebook](./import_to_console.ipynb) and run the cells.

You can run all cells at once, or you can run the cells one by one.

If successful, "**Edge Application**" will be imported and converted in the "**Console for AITRIOS**" and the following log will be displayed:
```bash
Converting...
	app_name: xxxxxxxxx
	version_number: xxxxxxxxx
```

Converting "**Edge Application**" will take some time, so run the "Get "**Edge Application**" status" cell to check the result of conversion.

When the "Get "**Edge Application**" status" cell is executed, the conversion status of each device is displayed as follows:
```bash
Conversion completed
	app_name: xxxxxxxxx
	version_number: xxxxxxxxx
```
The status is "Converting...", "Conversion completed", "Conversion failed".