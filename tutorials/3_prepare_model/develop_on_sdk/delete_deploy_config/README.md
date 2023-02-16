# Delete deploy configuration

This tutorial shows how to delete a deploy configuration in Console for AITRIOS.

## Get started

### 1. Set up Console Access Library
Set up access library client to delete a deploy configuration.

See [README](../../../_common/set_up_console_client/README.md) for details.

### 2. Get deploy configuration list (optional)
You need **`config_id`** of the deploy configuration that you want to delete.
If you want to get information for deployment configurations registerd in Console for AITRIOS, get the configuration list and check the config ID.

See [README](../get_deploy_config/README.md) for details.

### 3. Create setting file
Place setting file (**`./configuration.json`** file) for deleting. 
- configuration.json
    ```json
    {
        "config_id": ""
    }
    ```
> **NOTE**<br>
> See [4. Edit settings](#4-edit-settings) for details on the parameter.

### 4. Edit settings
Edit the parameters in [configuration.json](./configuration.json).

The parameters required to run this notebook are :

|Setting|Description|Range|Required/Optional|Remarks
|:--|:--|:--|:--|:--|
|**`config_id`**|The ID of the deploy configuration you want to delete.|String.<br>See NOTE.|Required|Used for Console Access Library API:<br>**`deployment.deployment.Deployment.delete_deploy_configuration`**|

> **NOTE**
>
> See [API Reference](https://developer.aitrios.sony-semicon.com/development-guides/reference/api-references/) of Console Access Library for other restrictions.


### 5. Run the notebook
Open the [notebook](./delete_deploy_config.ipynb) and run the cells.

You can run all cells at once, or you can run the cells one by one.

If successful, the deployment configuration will be deleted and the following log will be displayed:

```
The configuration was deleted successfully.
```
