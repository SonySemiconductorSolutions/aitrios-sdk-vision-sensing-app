# Setup "**Console Access Library**"
The SDK provides a Jupyter Notebook for setting up the "**Console Access Library**". <br>
By running the [set_up_console_client.ipynb](./set_up_console_client.ipynb), you will be able to use the "**Console Access Library**" on other notebooks in the SDK.

## Get started
### 1. Get access information
To use the "**Console Access Library**", you need following information to access the "**Console**".

- **`console_endpoint`**
- **`portal_authorization_endpoint`**
  - See ["**REST API Authentication**"](https://developer.aitrios.sony-semicon.com/file/download/rest-api-authentication) for more information on how to obtain these endpoints.

- **`client_secret`**
- **`client_id`**
  - You can obtain Client Secret and Client ID on "**Portal for AITRIOS**". <br>
  See ["**Portal for AITRIOS User Manual**"](https://developer.aitrios.sony-semicon.com/en/documents/portal-user-manual) for details.

### 2. Create setting file
Place setting file (**`./configuration.json`** file) for using the "**Console Access Library**". 
- configuration.json
    ```json
    {
        "console_endpoint" : "",
        "portal_authorization_endpoint" : "",
        "client_secret" : "",
        "client_id" : ""
    }
    ```
> **NOTE**<br>
> See [3. Edit settings](#3-edit-settings) for details on the parameter.

### 3. Edit settings

Edit the parameters in [configuration.json](./configuration.json).

|Setting|Range|Required/Optional|Remarks
|:--|:--|:--|:--|
|**`console_endpoint`**|String. See NOTE.|Required|Used for "**Console Access Library**" API: **`common.config.Config`**
|**`portal_authorization_endpoint`**|String. See NOTE.|Required|Used for "**Console Access Library**" API: **`common.config.Config`**
|**`client_secret`**|String. See NOTE.|Required|Used for "**Console Access Library**" API: **`common.config.Config`**
|**`client_id`**|String. See NOTE.|Required|Used for "**Console Access Library**" API: **`common.config.Config`**

> **NOTE**
> See [API Reference](https://developer.aitrios.sony-semicon.com/development-guides/reference/api-references/) of "**Console Access Library**" for other restrictions.

### 4. Run the notebook
Open [notebook](./set_up_console_client.ipynb) and run the cells.

You can run all cells at once, or you can run the cells one by one.

If successful, access library client will be created and the following log will be displayed:
```bash
Stored 'client_obj' (Client)
Console access lib client setup complete.
```
Then, you can use "**Console Access Library**" from other notebooks.

