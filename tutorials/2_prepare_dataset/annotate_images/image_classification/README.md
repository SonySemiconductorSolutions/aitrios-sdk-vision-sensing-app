# Tag Images with CVAT

This tutorial shows you how to annotate your images using [CVAT](https://github.com/opencv/cvat).

You can generate image classification dataset used for transfer learning in "**Edge Application SDK**".

## 1. Start up CVAT

1. In the VS Code TERMINAL tab, run following.

    ```
    .devcontainer/cvat/startup.sh
    ```

    After a few minutes, in the VS Code PORTS tab, "CVAT Web UI" 8080 port will be activated.

    **NOTE**

    CVAT stops when Codespaces (devcontainer) stops. If you want to stop CVAT manually, in the VS Code TERMINAL tab, run following.

    ```
    .devcontainer/cvat/stop.sh
    ```

2. In the VS Code PORTS tab, open the 8080 port in web browser.

    **NOTE**
    
    Sometimes 400 bad request error is displayed by Codespaces. Then close the error page of web browser and re-open the 8080 port in web browser.

    **NOTE**

    If 502 bad gateway error is displayed on the CVAT Web UI login screen, please wait a minute and go to the next step.

3. (Only at first time) Create superuser of CVAT.

   In the VS Code TERMINAL tab, do the following to add the superuser to CVAT, and enter the username, email, and password.

    ```
    .devcontainer/cvat/create_superuser.sh
    ```

4. (Only not yet logged in) In the CVAT Web UI login screen, enter the username and password of CVAT superuser, and sign in.

    **NOTE**

    If 502 bad gateway error is displayed, please wait a minute and retry to sign in.

## 2. Import images

The procedure is described below. See the [CVAT Documentation](https://opencv.github.io/cvat/docs/) for details.

JPEG image format is supported in "**Edge Application SDK**".

A number of images each label is recommended at least 10 to use for training AI Model.

If you want to import images from local machine running web browser, please see following [Import images from local machine using CVAT Web UI](#import-images-from-local-machine-using-cvat-web-ui).

If you want to import images from Codespaces (devcontainer), please see following [Import images from Codespaces (devcontainer)](#import-images-from-codespaces-devcontainer).

### Import images from local machine using CVAT Web UI

Please see details on [Getting started in CVAT](https://opencv.github.io/cvat/docs/getting_started/#getting-started-in-cvat)

1. In CVAT Web UI, create new project.
2. Open the project, and add labels.
3. In the project, create new task.
4. Select images or zip file to import.
5. Press "Submit & Open" button.

> **NOTE**
> 
> There is a following restriction when opening Codespaces in web browser.
> - Importing file larger than 2MB will fail. As workaround, please open Codespaces in VS Code or do following [Import images from Codespaces (devcontainer)](#import-images-from-codespaces-devcontainer).

### Import images from Codespaces (devcontainer)

1. In CVAT Web UI, create new project.
2. Open the project, and add labels.
3. Place setting file (**`./configuration.json`** file) for importing images. 
    - configuration.json
        ```json
        {
            "cvat_username": "django",
            "cvat_password": "",
            "cvat_project_id": 1,
            "import_dir": "./images",
            "import_image_extension": "jpg",
            "import_task_name": "your_task_name"
        }
        ```
    > **NOTE**<br>
    > See step 4 for details on the parameter.

4. In VS Code, edit the parameters in [configuration.json](./configuration.json).

    You need to identify a username and password to sign in to the CVAT.

    You need to identify the project when you import images.

    |Setting|Description|Range|Required/Optional
    |:--|:--|:--|:--|
    |**`cvat_username`**|Username to sign in to the CVAT|string (typical: django)|Required|
    |**`cvat_password`**|Password to sign in to the CVAT|string|Required|
    |**`cvat_project_id`**|Project id to import images into CVAT|integer|Required|
    |**`import_dir`**|Folder path for import files|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|
    |**`import_image_extension`**|Extension of JPEG images for import files|string of extension (typical: jpg)|Required|
    |**`import_task_name`**|Task name of CVAT to be created by import files. Importing with the same task name more than once will create tasks with the same name (but different task IDs). |string|Required|
    
>**NOTE**
>
>**`cvat_project_id`** The project id is displayed in the CVAT Web UI project page like 
>
>"Project #1 created by USERNAME on CREATED_DATE" (in this case the project id is 1).

5. Open [Notebook](./import_api.ipynb) and run all cells.

    If succeeded, "Project and Task linking completed." message appears.

## 3. Annotate images

1. In CVAT Web UI, open the task.
2. Change drop-down menu value from "Standard" to "Tag annotation".
3. Add label to the images.
4. Press "≡" (menu) button and press "Finish the job" button.

Please see also [Annotation with tags in CVAT](https://opencv.github.io/cvat/docs/manual/advanced/annotation-with-tags/)

## 4. Export dataset through CVAT API

If you want to export dataset to local machine running web browser, please see following [Export dataset to local machine using CVAT Web UI](#export-dataset-to-local-machine-using-cvat-web-ui).

If you want to export dataset to Codespaces (devcontainer), please see following [Export dataset to Codespaces (devcontainer)](#export-dataset-to-codespaces-devcontainer).

### Export dataset to local machine using CVAT Web UI

1. In CVAT Web UI, open the project.
2. Press "⁝" (menu) button, then select "Export dataset".
3. Select "ImageNet 1.0" format.
4. Check "Save images".
5. Press "OK" button and save to file.

### Export dataset to Codespaces (devcontainer)

1. Place setting file (**`./configuration.json`** file) for exporting dataset. 
    - configuration.json
        ```json
        {
            "cvat_username": "django",
            "cvat_password": "",
            "cvat_project_id": 1,
            "export_format": "ImageNet 1.0",
            "export_dir": "../../../_common/dataset/"
        }
        ```
    > **NOTE**<br>
    > See step 2 for details on the parameter.

2. In VS Code, edit the parameters in [configuration.json](./configuration.json).

    You need to identify a username and password to sign in to the CVAT.

    You need to identify the project when you export dataset.

    |Setting|Description|Range|Required/Optional
    |:--|:--|:--|:--|
    |**`cvat_username`**|Username to sign in to the CVAT|string (typical: django)|Required|
    |**`cvat_password`**|Password to sign in to the CVAT|string|Required|
    |**`cvat_project_id`**|Project id to export|integer|Required|
    |**`export_format`**|CVAT Output Format|Supports only fixed-value "ImageNet 1.0"|Required|
    |**`export_dir`**|Destination path to export annotation information from CVAT|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|

>**NOTE**
>
>- **`cvat_project_id`** The project id is displayed in the CVAT Web UI project page like "Project #1 created by USERNAME on CREATED_DATE" (in this case the project id is 1).
>
>- **`export_dir`** If you specify a path that doesn't exist, a new folder will be created.

3. Open [Notebook](./export_api.ipynb) and run all cells.

    If succeeded, "Dataset moved. Download process is completed." message appears. And exported zip file is saved to the **`export_dir`** folder
    
    If zip file already exists in **`export_dir`**, "Failed to move dataset. A dataset file with the same name exists." message appears and the zip file is not exported.

## 5. Convert dataset to use for "**Edge Application SDK**"

1. Place setting file (**`./configuration.json`** file) for conversion dataset. 
    - configuration.json
        ```json
        {
            "dataset_conversion_base_file": "../../../_common/dataset/project_id_1.zip",
            "dataset_conversion_dir": "../../../_common/dataset/",
            "dataset_conversion_validation_split": 0.2,
            "dataset_conversion_seed": 123
        }
        ```
    > **NOTE**<br>
    > See step 2 for details on the parameter.

2. In VS Code, edit the parameters in [configuration.json](./configuration.json).

    You need to identify a `export_dir` that contains zip file of dataset.

    You need to identify conversion parameters.

    |Setting|Description|Range|Required/Optional
    |:--|:--|:--|:--|
    |**`dataset_conversion_base_file`**|File path of dataset to convert (A zip file exported from CVAT for use in SDK's AI model learning and quantization)|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|
    |**`dataset_conversion_dir`**|Destination path to format the annotation information exported from CVAT for use in SDK's AI model learning and quantization. If the folder already contains the dataset, conversion will stop with error message.|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|
    |**`dataset_conversion_validation_split`**|The percentage of dataset images that are not used for training but are used for validation when the dataset is converted to format.|greater than 0.0 - less than 1.0 (typical: 0.2)|Required|
    |**`dataset_conversion_seed`**|Random seed value for shuffling dataset images during format conversion of the dataset| 0 - 4294967295 integers (typical: 123)|Required|
    
>**NOTE**
>
> **`dataset_conversion_dir`** If you specify a path that doesn't exist, a new folder will be created.

3. Open [Notebook](./convert_dataset.ipynb) and run all cells.

    If succeeded, "XX training files coped. XX validation files coped." message appears. And the dataset is splitted to train and validate into the **`dataset_conversion_dir`** folder
