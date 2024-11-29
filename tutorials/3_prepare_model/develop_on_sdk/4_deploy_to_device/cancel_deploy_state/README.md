# Cancel deploy status

This tutorial shows how to cancel deploying status.

> **NOTE**
>
> This feature is intended to be used when a device hangs after you start deploying a AI model and the deployment state on the database remains "Deploying".
> The operation is not guaranteed if this function is executed during normal operation.

## Get started
### 1. Set up "**Console Access Library**"
Set up access library client to cancel deploying status.

See [README](../../../../_common/set_up_console_client/README.md) for details.

### 2. Get deployment information (optional)
You need **`device_id`** and **`deploy_id`** of the deployment that you want to cancel "Deploying" status.

If you want to get information on your deployment, get the list of deployment information by running "Check deployment status" cell in the [notebook](../deploy_to_device/deploy_to_device.ipynb) for deploying model and check the device ID and deploy ID.

See [README](../deploy_to_device/README.md) for details.

### 3. Create setting file
Place setting file (**`./configuration.json`** file) for state cancellation. 
- configuration.json
    ```json
    {
        "device_id": "",
        "deploy_id" : ""
    }
    ```
> **NOTE**<br>
> See [4. Edit settings](#4-edit-settings) for details on the parameter.

### 4. Edit settings
Edit the parameters in [configuration.json](./configuration.json).

The parameters required to run this notebook are :

|Setting|Description|Range|Required/Optional|Remarks
|:--|:--|:--|:--|:--|
|**`device_id`**|The ID of the device you want to cancel the deployment.|String.<br>See NOTE.|Required|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.cancel_deployment`**|
|**`deploy_id`**|The ID of the deployment you want to cancel.|String.<br>See NOTE.|Required|Used for "**Console Access Library**" API:<br>**`deployment.deployment.Deployment.cancel_deployment`**|

> **NOTE**
>
> See [API Reference](https://developer.aitrios.sony-semicon.com/en/edge-ai-sensing/guides/) of "**Console Access Library**" for other restrictions.

### 5. Run the notebook
Open the [notebook](./cancel_deploy_state.ipynb) and run the cells.

You can run all cells at once, or you can run the cells one by one.

If successful, the deployment status will be "Cancel" and the following log will be displayed:

```
Deployment status changed to 'Cancel'.
```