set_languages("c++20")
add_rules("mode.debug", "mode.release")

add_requires("cuda", {system = true})

option("enable_example", function()
    set_default(false)
    set_showmenu(true)
    set_description("Enable building example program.")
end)

target("codec", function()
    set_kind("static")
    
    add_includedirs("include")
    add_includedirs("src")
    add_includedirs("src/Utils")
    add_includedirs("src/Interface")
    add_includedirs("src/NvEncoder")
    add_includedirs("src/NvDecoder")
    add_includedirs("src/codec")
    
    add_files("src/*.cpp")
    add_files("src/codec/*.cpp")
    add_files("src/NvEncoder/*.cpp|NvEncoderGL.cpp")
    add_files("src/NvDecoder/*.cpp")
    
    add_files("src/Utils/*.cu")

    add_cxxflags("/FS")
    
    -- CUDA configurations
    add_cuflags("-arch=sm_120")-- Adjust this based on your GPU architecture
    -- add_cugencodes("sm_120")-- Adjust this based on your GPU architecture
    
    -- Enable CUDA device linking
    set_policy("build.cuda.devlink", true)
    
    if is_plat("windows") then
        add_defines("WIN32")
        add_links("nvcuvid", "nvencodeapi", "cudart", "cuda", "cublas", "curand")
        add_linkdirs("libs", "$(env CUDA_PATH)/lib/x64")
    end
    
    -- Add CUDA support
    if has_config("cuda") then
        add_rules("cuda")
        add_packages("cuda")
    end
end)

if has_config("enable_example") then
    target("codec_example", function()
        set_kind("binary")
        add_includedirs("include")
        add_files("src/example.cpp")
        add_deps("codec")
        
        if is_plat("windows") then
            add_links("nvcuvid", "nvencodeapi", "cudart", "cuda")
            add_linkdirs("libs", "$(env CUDA_PATH)/lib/x64")
        end
        
        if has_config("cuda") then
            add_packages("cuda")
        end
    end)
end
