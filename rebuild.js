var cmakeJS = require("cmake-js");

var options = {
    runtime: process.env.npm_config_wcjs_runtime || null,
    runtimeVersion: process.env.npm_config_wcjs_runtime_version || null,
    arch: process.env.npm_config_wcjs_arch || null
};

var buildSystem = new cmakeJS.BuildSystem(options);
buildSystem.rebuild();
