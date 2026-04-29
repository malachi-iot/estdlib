# Boilerplate from:
# https://docs.espressif.com/projects/esp-idf/en/v6.0.1/esp32/api-guides/tools/idf-py.html
# https://developer.espressif.com/blog/2025/10/idf_py_extension/

#from typing import Any
import click

def action_extensions(base_actions: dict, project_path: str) -> dict:
    def global_callback_detail(ctx: click.Context, global_args: dict, tasks: list) -> None:
        if getattr(global_args, 'detail', False):
            print(f"About to execute {len(tasks)} task(s): {[t.name for t in tasks]}")

    return {
        # TODO: Put a "real fullclean in here"
        "version": "1",
        "global_options": [
            {
                "names": ["--detail", "-d"],
                "help": "Enable detailed output"
            }
        ],
        "global_action_callbacks": [global_callback_detail]
    }