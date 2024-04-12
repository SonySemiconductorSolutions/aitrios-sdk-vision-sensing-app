# Copyright 2023 Sony Semiconductor Solutions Corp. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import errno
import json
import jsonschema
import pathlib
import sys
import os


def validate_symlink(path: pathlib.Path):
    if path.is_symlink():
        raise OSError(
            errno.ELOOP,
            "Symbolic link is not supported. Please use real folder or file",
            f"{path}",
        )


def main():
    json_path = pathlib.Path(os.path.dirname(__file__) + "/testapp_configuration.json")
    validate_symlink(json_path)

    with open(json_path, "r") as f:
        senscord_default_property = json.load(f)

    schema_json_path = pathlib.Path(
        os.path.dirname(__file__) + "/testapp_configuration_schema.json"
    )
    validate_symlink(schema_json_path)

    with open(schema_json_path, "r") as f:
        json_schema = json.load(f)

    # validate with jsonschema file
    try:
        jsonschema.validate(senscord_default_property, json_schema)
    except jsonschema.ValidationError as e:
        print(e)
        sys.exit(1)


if __name__ == "__main__":
    main()
