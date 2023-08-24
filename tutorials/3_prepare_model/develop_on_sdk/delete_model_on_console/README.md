# Delete AI model from "**Console for AITRIOS**"
The SDK provides a Jupyter Notebook for deleting AI model from "**Console for AITRIOS**". <br>
By running the [delete_model_on_console.ipynb](./delete_model_on_console.ipynb), you can delete the AI model from the "**Console**".

## Get started
### 1. Setup "**Console Access Library**"
To import the AI model into the "**Console for AITRIOS**", setup access library client.

See [README](./../../../_common/set_up_console_client/README.md) to get started.

### 2. Get model list
If you want to upgrade an AI model, get the model list and check the model ID and version number.

See [README](./../get_model_list/README.md) to get started.

### 3. Create setting file
Place setting file (**`./configuration.json`** file) for deleting. 
- configuration.json
    ```json
	{
		"model_id": ""
	}
    ```
> **NOTE**<br>
> See [4. Edit settings](#4-edit-settings) for details on the parameter.

### 4. Edit settings
Edit the parameters in [configuration.json](./configuration.json).

The parameters required to run this notebook are :
|Setting|Description|Range|Required/Optional|Remarks
|:--|:--|:--|:--|:--|
|**`model_id`**|The ID of the AI model you want to delete |String. <br>See NOTE. |Required|Used for "**Console Access Library**" API:<br>**`ai_model.ai_model.AIModel.delete_model`**|

> **NOTE**<br>
> See [API Reference](https://developer.aitrios.sony-semicon.com/development-guides/reference/api-references/) of "**Console Access Library**" for other restrictions.

### 5. Run the notebook
Open [notebook](./delete_model_on_console.ipynb) and run the cells.

You can run all cells at once, or you can run the cells one by one.

If successful, AI model will be removed from the "**Console for AITRIOS**" and the following log will be displayed:
```bash
Deleting AI model is completed 
	model_id: xxxxxxxxx
```

