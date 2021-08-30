//
// Created by 陈润泽 on 2021/8/25.
//

#include "geyser.h"
#include "fmt/format.h"
#include <filesystem>
#include <functional>

using namespace py::literals;

bool geyser::Geyser::python_mode = false;
py::module_ geyser::Geyser::sys;
py::type geyser::Geyser::Bucket = py::module_::import("geyserpy.bucket").attr("Bucket").cast<py::type>();

void geyser::Geyser::init() {
    sys = py::module_::import("sys");

    Logger::init();
    Profile::init();
}

void geyser::Geyser::bind(py::class_<Geyser> &&clazz) {
    clazz.def_static("entry", py::overload_cast<>(&Geyser::entry));
    clazz.def_static("composable", py::overload_cast<bool>(&Geyser::composable));
    clazz.def_static("composable", py::overload_cast<py::object, const std::string &>(&Geyser::composable));
    clazz.def_static("composable", py::overload_cast<const std::string &, bool>(&Geyser::composable));
    clazz.def_static("composable", py::overload_cast<py::object>(&Geyser::composable));
    clazz.def_static("composable", py::overload_cast<>(&Geyser::composable));
    clazz.def_static("executable", py::overload_cast<const std::string &>(&Geyser::executable));
    clazz.def_static("executable", py::overload_cast<py::object, const std::string &>(&Geyser::executable));
    clazz.def_static("executable", py::overload_cast<>(&Geyser::executable));
}

int geyser::Geyser::entry() {
    Geyser::python_mode = true;
    Geyser::entry(0, nullptr, nullptr);
    return 0;
}


int geyser::Geyser::entry(int argc, const char **argv, const char **envp) {
    auto logger = geyser::Logger::get("geyserpy.Geyser");
    print_runtime_info(logger);
    auto cmdl = Geyser::build_parser(argc, argv);
    auto profile_paths = Geyser::get_profile_paths(cmdl);
    for (const auto &path : profile_paths) {
        logger.info(fmt::format("Execute profile {}", path));
        geyser::Kernel kernel;
        if (std::filesystem::exists(path)) {
            auto profile = Profile::parse(path);
            if (profile.contains("__compose__"))
                kernel.compose_all(profile["__compose__"]);
            else
                throw py::value_error(fmt::format("Composable objects are undefined"));
        } else
            logger.error(fmt::format("Profile {} does NOT exists", path));
    }

    return 0;
}

void geyser::Geyser::print_runtime_info(geyser::Logger &logger) {
    auto platform = py::module_::import("platform");
    logger.info(fmt::format("geyserpy {}", GEYSER_MACRO_STRINGIFY(GEYSER_VERSION_INFO)));
    logger.info(fmt::format(
            "{} {} {} {} {} {} {}",
            GEYSER_MACRO_STRINGIFY(GEYSER_SYSTEM_NAME),
            GEYSER_MACRO_STRINGIFY(GEYSER_SYSTEM_VERSION),
            GEYSER_MACRO_STRINGIFY(GEYSER_SYSTEM_PROCESSOR),
            GEYSER_MACRO_STRINGIFY(GEYSER_COMPILER_NAME),
            GEYSER_MACRO_STRINGIFY(GEYSER_COMPILER_VERSION),
            __DATE__, __TIME__
    ));
    logger.info(fmt::format(
            "python {} {} ",
            py::str(platform.attr("python_version")()).cast<std::string>(),
            platform.attr("python_compiler")().cast<py::str>().cast<std::string>()
    ));
    logger.info(platform.attr("version")().cast<py::str>().cast<std::string>());

}

argh::parser geyser::Geyser::build_parser(int argc, const char **argv) {
    argh::parser parser;
    define_parser(parser);
    if (python_mode) {
        auto pyargv = sys.attr("argv").cast<py::list>();
        typedef const char *cstring;
        auto vargv = new cstring[py::len(pyargv)];
        for (auto idx = 0; idx < py::len(pyargv); ++idx)
            vargv[idx] = pyargv[idx].cast<std::string>().c_str();
        parser.parse(py::len(pyargv), argv);
    } else {
        parser.parse(argc, argv);
    }
    return parser;
}

void geyser::Geyser::define_parser(argh::parser &parser) {

}

std::vector<std::string> geyser::Geyser::get_profile_paths(argh::parser &parser) {
    std::vector<std::string> profile_paths;
    auto begin_idx = 0;
    if (Geyser::python_mode)
        begin_idx = 2;
    else
        begin_idx = 1;
    for (auto idx = begin_idx; idx < parser.pos_args().size(); ++idx)
        profile_paths.push_back(parser.pos_args()[idx]);
    return profile_paths;
}

std::function<py::object(py::object)> geyser::Geyser::composable(const std::string &name, bool auto_compose = true) {
    return [&](py::object clz) {
        auto bucket = Geyser::Bucket.attr("create")(
                "clazz"_a = clz, "name"_a = name, "auto_compose"_a = auto_compose,
                "safe_compose"_a = false
        );
        Kernel::register_class(name, bucket);
        return clz;
    };
}

std::function<py::object(py::object)> geyser::Geyser::composable(bool auto_compose = true) {
    return [&](py::object clz) {
        auto name = clz.attr("__name__").cast<py::str>().cast<std::string>();
        auto bucket = Geyser::Bucket.attr("create")(
                "clazz"_a = clz, "name"_a = name, "auto_compose"_a = auto_compose,
                "safe_compose"_a = false
        );
        Kernel::register_class(name, bucket);
        return clz;
    };
}

std::function<py::object(py::object)> geyser::Geyser::composable() {
    return Geyser::composable(true);
}

void geyser::Geyser::composable(py::object clazz, const std::string &name) {
    auto bucket = Geyser::Bucket.attr("create")(
            "clazz"_a = clazz, "name"_a = name, "auto_compose"_a = false,
            "safe_compose"_a = true
    );
    Kernel::register_class(name, bucket);
}

void geyser::Geyser::composable(py::object clazz) {
    Geyser::composable(clazz, clazz.attr("__name__").cast<py::str>().cast<std::string>());
}

std::function<py::object(py::object)> geyser::Geyser::executable(const std::string &name) {
    return [&](py::object func) {
        auto bucket = Geyser::Bucket.attr("create")(
                "clazz"_a = func, "name"_a = name, "auto_compose"_a = false,
                "safe_compose"_a = false
        );
        Kernel::register_class(name, bucket);
        return func;
    };
}

void geyser::Geyser::executable(py::object func, const std::string &name) {
    auto bucket = Geyser::Bucket.attr("create")(
            "clazz"_a = func, "name"_a = name, "auto_compose"_a = false,
            "safe_compose"_a = false
    );
    Kernel::register_class(name, bucket);
}

std::function<py::object(py::object)> geyser::Geyser::executable() {
    return [&](py::object func) {
        auto words = func.attr("__name__").attr("split")("_").cast<py::list>();
        py::list capwords;
        for (auto it : words)
            capwords.append(it.attr("capitalize")());
        auto name = ""_s.attr("join")(capwords);
        auto bucket = Geyser::Bucket.attr("create")(
                "clazz"_a = func, "name"_a = name, "auto_compose"_a = false,
                "safe_compose"_a = false
        );
        return func;
    };
}