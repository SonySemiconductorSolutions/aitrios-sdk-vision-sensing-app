# Download Images
This tutorial shows how to download images from [COCO](https://cocodataset.org/#home).

## Getting Started
### 1. Download annotation file from COCO
1. Download [2017 Train/Val annotations [241MB]](http://images.cocodataset.org/annotations/annotations_trainval2017.zip) from [COCO](https://cocodataset.org/#download) and decompress it.
2. Place **`instances_val2017.json`** in [annotations](./annotations/) folder.

### 2. Create setting file
Place setting file (**`./configuration.json`** file) for downloading images. 
- configuration.json
    ```json
    {
        "annotation_file": "./annotations/instances_val2017.json",
        "category_names": ["cat", "dog"],
        "max_download_count": 15,
        "licenses": [4, 5, 6],
        "remove_categories": ["person"],
        "output_dir": "./output"
    }
    ```

- configuration.json (example without optional parameters)
    ```json
    {
        "annotation_file": "./annotations/instances_val2017.json",
        "output_dir": "./output"
    }
    ```	

> **NOTE**<br>
> See [3. Edit settings](#3-edit-settings) for details on the parameter.

### 3. Edit settings
Edit the parameters in [configuration.json](./configuration.json).

The parameters required to run this notebook are :

|Setting|Description|Range|Required/Optional
|:--|:--|:--|:--|
|**`annotation_file`**|Path to COCO annotation file|Absolute path or relative path from configuration.json/Notebook (*.ipynb) |Required|
|**`category_names`**|Image categories to download|["Category1", "Category2", ...] *1 |Optional<br> If omitted or given an empty list, all categories will be downloaded.
|**`max_download_count`**|Max number of downloads for each category|0: unlimited (as many as possible)<br> 1 - Total number of images: Up to the specified number for each category<br><br>*If category_names is omitted or empty, this parameter will be a number of total downloads, not per category.|Optional<br> If omitted, the number of downloads is unlimited (as many as possible) |
|**`licenses`**|Licenses of images to download|[License1 ID, License2 ID, ・・・] *2|Optional<br>If omitted or given an empty list, images for all licenses will be downloaded.|
|**`remove_categories`**|Image categories to exclude|["Category1", "Category2", ...] *1 |Optional<br>If omitted or given an empty list, no categories will be excluded.|
|**`output_dir`**|Path to output downloaded images (a new directory will be created if it does not exist)|Absolute path or relative path from configuration.json/Notebook (*.ipynb)|Required|


*1 Select category name(s) from the following table.

|type:|Person|Vehicle|Outdoor|Animal|Accessory|Sports|Kitchen|Food|Furniture|Electronic|Appliance|Indoor|
|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|:--|
|**category name**|person|bicycle|traffic light|bird|backpack|frisbee|bottle|banana|chair|tv|microwave|book|
|||car|fire hydrant|cat|umbrella|skis|wine glass|apple|couch|laptop|oven|clock|
|||motorcycle|stop sign|dog|handbag|snowboard|cup|sandwich|potted plant|mouse|toaster|vase|
|||airplane|parking meter|horse|tie|sports ball|fork|orange|bed|remote|sink|scissors|
|||bus|bench|sheep|suitcase|kite|knife|broccoli|dining table|keyboard|refrigerator|teddy bear|
|||train||cow||baseball bat|spoon|carrot|toilet|cell phone||hair drier|
|||truck||elephant||baseball glove|bowl|hot dog||||toothbrush|
|||boat||bear||skateboard||pizza|||||
|||||zebra||surfboard||donut|||||
|||||giraffe||tennis racket||cake|||||


*2 Select license(s) from the following table.
|License|ID|
|:--|:--|
|[Attribution-NonCommercial-ShareAlike License](https://creativecommons.org/licenses/by-nc-sa/2.0/) (CC BY-NC-SA 2.0)|1|
|[Attribution-NonCommercial License](https://creativecommons.org/licenses/by-nc/2.0/) (CC BY-NC 2.0)|2|
|[Attribution-NonCommercial-NoDerivs License](http://creativecommons.org/licenses/by-nc-nd/2.0/) (CC BY-NC-ND 2.0)|3|
|[Attribution License](http://creativecommons.org/licenses/by/2.0/) (CC BY 2.0)|4|
|[Attribution-ShareAlike License](http://creativecommons.org/licenses/by-sa/2.0/) (CC BY-SA 2.0)|5|
|[Attribution-NoDerivs License](http://creativecommons.org/licenses/by-nd/2.0/) (CC BY-ND 2.0)|6|
|[No known copyright restrictions](http://flickr.com/commons/usage/)|7|
|[United States Government Work](http://www.usa.gov/copyright.shtml)|8|

### 4. Run the notebook
Open [notebook](./get_dataset_images_from_coco.ipynb) and run the cells.

If successful, images will be downloaded in **`output_dir`**.

You can run all cells at once, or you can run the cells one by one.

### 5. (Optional) Import COCO images to "**Console for AITRIOS**"

If you want to use the downloaded COCO images to train or retrain AI model on "**Console for AITRIOS**",

see ["**Console Manual**"](https://developer.aitrios.sony-semicon.com/en/documents/console-user-manual) for details.

- Create model
- Train model
- Labeling & Training
