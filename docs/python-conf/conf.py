import datetime
from flowerevolver import __version__ as release

project = "FlowerEvolver-WASM"
copyright = f"2023-{datetime.datetime.now().year}, Cristian Gonzalez"
author = "Cristian Gonzalez"

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.autosectionlabel",
    "sphinx.ext.githubpages",
]

autodoc_member_order = "bysource"
autodoc_default_options = {
    "members": True,
    "undoc-members": True,
    "imported-members": True,
    "show-inheritance": True,
}
autosummary_generate = True

templates_path = []
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

primary_domain = "py"
highlight_language = "python"

html_theme = "pydata_sphinx_theme"
html_theme_options = {
    "icon_links": [
        {
            "name": "GitHub",
            "url": "https://github.com/cristianglezm/FlowerEvolver-WASM",
            "icon": "fa-brands fa-square-github",
        },
        {
            "name": "PyPI",
            "url": "https://pypi.org/project/flowerevolver/",
            "icon": "fa-brands fa-python",
        },
    ],
    "external_links": [
        {"name": "Demo", "url": "https://cristianglezm.github.io/FlowerEvolver-WASM/"},
        {"name": "C++ Docs", "url": "https://cristianglezm.github.io/FlowerEvolver-WASM/docs/cpp/"},
        {"name": "JS/WASM Docs", "url": "https://cristianglezm.github.io/FlowerEvolver-WASM/docs/js/"},
    ],
    "show_toc_level": 2,
    "navigation_depth": 3,
    "navbar_end": ["navbar-icon-links"],
}
html_title = f"FlowerEvolver-WASM {release} Python API"
# served at /docs/python/ alongside the demo at / - see .github/workflows/cd.yml
html_baseurl = "https://cristianglezm.github.io/FlowerEvolver-WASM/docs/python/"
html_static_path = ["_static"]
html_css_files = ["custom.css"]
