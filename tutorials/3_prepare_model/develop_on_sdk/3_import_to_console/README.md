# Import AI model to "**Console for AITRIOS**"
The SDK provides a Jupyter Notebook to import AI model to "**Console for AITRIOS**". <br>
By running the [import_to_console.ipynb](./import_to_console.ipynb), you can import the AI model into the "**Console**" and you are ready to deploy it to the device.

> **NOTE**
>
> Azure Blob Storage is required to import an AI model to "**Console for AITRIOS**" using this notebook.<br>
> If you want to import an AI model from a local environment, use "**Console UI**". See ["**Console User Manual**"](https://developer.aitrios.sony-semicon.com/en/documents/console-user-manual) for details.

## Get started
### 1. Setup "**Console Access Library**"
To import the AI model into the "**Console for AITRIOS**", setup access library client.

See [README](./../../../_common/set_up_console_client/README.md) to get started.

### 2. Get model list (optional)
If you want to upgrade an AI model, get the model list and check the model ID and version number.

See [README](./../get_model_list/README.md) to get started.

### 3. Create setting file
Place setting file (**`./configuration.json`** file) for importing. 
- configuration.json
    ```json
	{
		"model_id": "",
		"model": "",
		"converted": false,
		"vendor_name": "",
		"comment": "",
		"network_type": "0"
	}
    ```

- configuration.json (example without optional parameters)
    ```json
	{
		"model_id": "",
		"model": ""
	}
    ```	

> **NOTE**<br>
> See [4. Edit settings](#4-edit-settings) for details on the parameter.

### 4. Edit settings
Edit the parameters in [configuration.json](./configuration.json).

The parameters required to run this notebook are :
|Setting|Description|Range|Required/Optional|Remarks
|:--|:--|:--|:--|:--|
|**`model_id`**|The ID of the AI model you want to import|String. <br>See NOTE. |Required|Used for "**Console Access Library**" API:<br>**`ai_model.ai_model.AIModel.import_base_model`**<br>**`ai_model.ai_model.AIModel.get_base_model_status`**<br>**`ai_model.ai_model.AIModel.publish_model`** |
|**`model`**|Path to SAS URI for AI model|SAS URI. <br>See NOTE. |Required|Used for "**Console Access Library**" API:<br>**`ai_model.ai_model.AIModel.import_base_model`**|
|**`converted`**|AI model converted flag <br>If set to false, the model will be converted at import on "**Console for AITRIOS**" |true or false. <br> (typical: false) <br>See NOTE. |Optional|Used for "**Console Access Library**" API:<br>**`ai_model.ai_model.AIModel.import_base_model`**|
|**`vendor_name`**|vendor name|String. <br>See NOTE. |Optional|Used for "**Console Access Library**" API:<br>**`ai_model.ai_model.AIModel.import_base_model`**|
|**`comment`**|Description of the AI model and version|String. <br>See NOTE. |Optional|Used for "**Console Access Library**" API:<br>**`ai_model.ai_model.AIModel.import_base_model`**|
|**`network_type`**|network type|String. <br>See NOTE. |Optional<br>If omitted, set default value:"0"  *1|Used for "**Console Access Library**" API:<br>**`ai_model.ai_model.AIModel.import_base_model`**|

*1 SDK suppose to use "**Edge Application**", therefore set default value automatically. 

> **NOTE**<br>
> See [API Reference](https://developer.aitrios.sony-semicon.com/development-guides/reference/api-references/) of "**Console Access Library**" for other restrictions.

### 5. Run the notebook
Open [notebook](./import_to_console.ipynb) and run the cells.

You can run all cells at once, or you can run the cells one by one.

If successful, AI model will be imported to and converted in the "**Console for AITRIOS**" and the following log will be displayed:
```bash
Converting... 
	model_id: xxxxxxxxx
```
Converting AI model will take some time, so run the "Get AI model status after conversion" cell to check the result of conversion.

When the "Get AI model status after conversion" cell is executed, the conversion status of each device is displayed as follows:
```bash
Conversion completed 
	model_id: xxxxxxxxx
```
The status is "Converting...", "Conversion completed", "Conversion failed".