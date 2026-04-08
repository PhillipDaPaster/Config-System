#pragma once
//#define CFG_USE_IMCOLOR // uncomment this if you would like to use imcolor 

#include <vector>
#include <typeinfo>
#include <fstream>
#include <iomanip>
#include <string>
#include <filesystem>

#include "json.h" // nlohmann::json

#ifdef CFG_USE_IMCOLOR
#include "imgui_internal.h" // put your projects file path here 
#endif

namespace Config {
    struct ConfigVarInitializer {
        std::vector<std::string> m_segments;
        void* m_ptr{ nullptr };
        size_t m_type_hash{ 0 };
        bool m_no_save{ false };
    };

    inline std::vector<ConfigVarInitializer> vars{};

    inline std::vector<std::string> SplitDotPath(const std::string& path) {
        std::vector<std::string> out;
        out.reserve(4);
        size_t start = 0;
        while (start < path.size()) {
            const size_t dot = path.find('.', start);
            const size_t end = (dot == std::string::npos) ? path.size() : dot;
            out.emplace_back(path.substr(start, end - start));
            if (dot == std::string::npos) 
                break;

            start = dot + 1;
        }
        return out;
    }

    inline std::string CppNsToJsonPath(std::string s) {
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); ) {
            if (s[i] == ' ') {
                ++i;
                continue;
            }
            if (i + 1 < s.size() && s[i] == ':' && s[i + 1] == ':') {
                out.push_back('.');
                i += 2;
                continue;
            }
            out.push_back(s[i]);
            ++i;
        }
        return out;
    }

    inline nlohmann::json* GetJsonNode(nlohmann::json& j, const std::vector<std::string>& segments) {
        nlohmann::json* current = &j;
        for (const std::string& seg : segments) {
            current = &((*current)[seg]);
        }
        return current;
    }

    inline nlohmann::json* FindJsonNode(nlohmann::json& j, const std::vector<std::string>& segments) {
        nlohmann::json* current = &j;
        for (const std::string& seg : segments) {
            auto it = current->find(seg);
            if (it == current->end())
                return nullptr;

            current = &(*it);
        }
        return current;
    }

    inline void Save(const std::string& filepath) {
        std::ofstream output_file(filepath);
        if (!output_file.is_open()) 
            return;

        nlohmann::json j;

        for (const auto& var : vars) {
            if (var.m_no_save) 
                continue;

            nlohmann::json* node = GetJsonNode(j, var.m_segments);

            if (var.m_type_hash == typeid(bool).hash_code())
                *node = *static_cast<bool*>(var.m_ptr);
            else if (var.m_type_hash == typeid(int).hash_code())
                *node = *static_cast<int*>(var.m_ptr);
            else if (var.m_type_hash == typeid(float).hash_code())
                *node = *static_cast<float*>(var.m_ptr);
#ifdef CFG_USE_IMCOLOR
            else if (var.m_type_hash == typeid(ImColor).hash_code()) {
                auto clr = *static_cast<ImColor*>(var.m_ptr);
                *node = { clr.Value.x, clr.Value.y, clr.Value.z, clr.Value.w };
            }
#endif
            else if (var.m_type_hash == typeid(std::string).hash_code())
                *node = *static_cast<std::string*>(var.m_ptr);
        }

        output_file << std::setw(4) << j;
    }

    inline void Load(const std::string& filepath) {
        if (!std::filesystem::exists(filepath) || std::filesystem::is_empty(filepath)) {
            return; 
        }

        std::ifstream input_file(filepath);
        if (!input_file.is_open()) 
            return;

        nlohmann::json j;
        try {
            input_file >> j;
        }
        catch (...) { // failed to parse json skipped loading config
            input_file.close();
            return;
        }

        for (Config::ConfigVarInitializer& var : vars) {
            if (var.m_no_save) 
                continue;

            nlohmann::json* node = FindJsonNode(j, var.m_segments);
            if (!node) 
                continue;

            if (var.m_type_hash == typeid(bool).hash_code())
                *static_cast<bool*>(var.m_ptr) = *node;
            else if (var.m_type_hash == typeid(int).hash_code())
                *static_cast<int*>(var.m_ptr) = *node;
            else if (var.m_type_hash == typeid(float).hash_code())
                *static_cast<float*>(var.m_ptr) = *node;
#ifdef CFG_USE_IMCOLOR
            else if (var.m_type_hash == typeid(ImColor).hash_code()) {
                if (node->is_array() && node->size() == 4) {
                    ImColor clr{
                        (*node)[0].get<float>(),
                        (*node)[1].get<float>(),
                        (*node)[2].get<float>(),
                        (*node)[3].get<float>()
                    };
                    *static_cast<ImColor*>(var.m_ptr) = clr;
                }
            }
#endif
            else if (var.m_type_hash == typeid(std::string).hash_code())
                *static_cast<std::string*>(var.m_ptr) = *node;
        }
    }
} // namespace Config

#define CFGVAR(cpp_namespacepath, configname, state) \
namespace cpp_namespacepath { \
    inline auto configname = state; \
    namespace configvar_initializers { \
        inline auto configname##_initializer = []() { \
            const std::string ns = Config::CppNsToJsonPath(#cpp_namespacepath); \
            auto segs = Config::SplitDotPath(ns); \
            segs.emplace_back(#configname); \
            Config::vars.push_back(Config::ConfigVarInitializer{ std::move(segs), &configname, typeid(configname).hash_code(), false }); \
            return true; \
        }(); \
    } \
}

#define CFGVAR_NOSAVE(cpp_namespacepath, configname, state) \
namespace cpp_namespacepath { \
    inline auto configname = state; \
    namespace configvar_initializers { \
        inline auto configname##_initializer = []() { \
            const std::string ns = Config::CppNsToJsonPath(#cpp_namespacepath); \
            auto segs = Config::SplitDotPath(ns); \
            segs.emplace_back(#configname); \
            Config::vars.push_back(Config::ConfigVarInitializer{ std::move(segs), &configname, typeid(configname).hash_code(), true }); \
            return true; \
        }(); \
    } \
}
