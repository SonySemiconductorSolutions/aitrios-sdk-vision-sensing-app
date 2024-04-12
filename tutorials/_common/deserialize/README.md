# Deserialize (for Inference result)
The SDK provides a Jupyter Notebook to deserialize for inference result. <br>
By running the [deserialize.ipynb](./deserialize.ipynb), you can deserialize the inference result.

## Get started
### 1. Place schema file and serialized file
Prepare a schema file for the serialized data and a file containing the serialized data in any directory under the **`deserialize`** directory.


### 2. (Only at first time) Build the deserialization environment
In the VS Code TERMINAL tab, run following.
```bash
$ ./tutorials/_common/deserialize/build.sh
```
After a few minutes, a docker image is created for deserialization.


### 3. Create setting file
Place setting file (**`./configuration.json`** file) for deserialize. 
- configuration.json
    ```json
	{
		"schema_file" : "",
		"serialized_file" : "",
		"input_type" : "json",
		"output_dir" : "./output/"
	}
    ```

> **NOTE**<br>
> See [4. Edit settings](#4-edit-settings) for details on the parameter.

### 4. Edit settings
Edit the parameters in [configuration.json](./configuration.json).

The parameters required to run this notebook are :
|Setting|Description|Range|Required/Optional|Remarks
|:--|:--|:--|:--|:--|
|**`schema_file`**|Path to schema file for serialized data|Absolute path or relative path from configuration.json/Notebook (*.ipynb) |Required||
|**`serialized_file`**|Path to file containing the serialized data|Absolute path or relative path from configuration.json/Notebook (*.ipynb) |Required||
|**`input_type`**|Type of **`serialized_file`**|"binary" or "json" |Required||
|**`output_dir`**|Path to output directory for deserialized file <br>  (a new directory will be created if it does not exist)|Absolute path or relative path from configuration.json/Notebook (*.ipynb) |Optional|If omitted or given an empty string, set path same as configuration.json/Notebook (*.ipynb). <br> The format of the output filename is "**`serialized_file`** without extension" + ".json".|


### 5. Run the notebook
Open [notebook](./deserialize.ipynb) and run the cells.

If successful, deserialized file will be output in **`output_dir`**.

You can run all cells at once, or you can run the cells one by one.


## References
- [FlatBuffers](https://google.github.io/flatbuffers/index.html)<br>
The version of FlatBuffers used in "**Edge Application SDK**" is 23.1.21.
