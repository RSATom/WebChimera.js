var cmakeJS = require("cmake-js");

var defaultRuntime = "nw";
var defaultRuntimeVersion = "0.12.2";
var defaultWinArch = "ia32";

var options = {
    runtime: process.env.npm_config_wcjs_runtime || undefined,
    runtimeVersion: process.env.npm_config_wcjs_runtime_version || undefined,
    arch: process.env.npm_config_wcjs_arch || undefined
};

var buildSystem = new cmakeJS.BuildSystem(options);

if(buildSystem.options.runtime == undefined) {
    buildSystem.options.runtime = defaultRuntime;
}

if(buildSystem.options.runtimeVersion == undefined) {
    buildSystem.options.runtimeVersion = defaultRuntimeVersion;
}

if(buildSystem.options.arch == undefined && process.platform == "win32") {
    buildSystem.options.arch = defaultWinArch;
}

buildSystem.rebuild();
