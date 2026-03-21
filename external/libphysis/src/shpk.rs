// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use physis::Platform;
use physis::ReadableFile;
use physis::shpk::MaterialParameter;
use physis::shpk::{Key, Node, Pass, ResourceParameter, ShaderPackage};
use std::ffi::c_char;
use std::ptr::{null, null_mut};
use std::{mem, slice};

use crate::{ffi_from_c_string, ffi_to_c_string, physis_Buffer};

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_ShaderParameter {
    slot: u16,
    name: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_Shader {
    len: u32,
    bytecode: *mut u8,
    num_scalar_parameters: u32,
    scalar_parameters: *mut physis_ShaderParameter,
    num_resource_parameters: u32,
    resource_parameters: *mut physis_ShaderParameter,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_SHPK {
    p_ptr: *mut ShaderPackage,
    num_vertex_shaders: u32,
    vertex_shaders: *mut physis_Shader,
    num_pixel_shaders: u32,
    pixel_shaders: *mut physis_Shader,

    num_system_keys: u32,
    system_keys: *mut Key,

    num_scene_keys: u32,
    scene_keys: *mut Key,

    num_material_keys: u32,
    material_keys: *mut Key,

    sub_view_key1_default: u32,
    sub_view_key2_default: u32,

    material_parameters_size: u32,
    num_material_parameters: u32,
    material_parameters: *mut MaterialParameter,

    material_default_parameters_size: u32,
    material_default_parameters: *mut f32,

    num_nodes: u32,
    nodes: *mut physis_SHPKNode,

    num_scalar_parameters: u32,
    scalar_parameters: *mut physis_ShaderParameter,
    num_texture_parameters: u32,
    texture_parameters: *mut physis_ShaderParameter,
}

impl Default for physis_SHPK {
    fn default() -> Self {
        Self {
            p_ptr: null_mut(),
            num_vertex_shaders: 0,
            vertex_shaders: null_mut(),
            num_pixel_shaders: 0,
            pixel_shaders: null_mut(),
            num_system_keys: 0,
            system_keys: null_mut(),
            num_scene_keys: 0,
            scene_keys: null_mut(),
            num_material_keys: 0,
            material_keys: null_mut(),
            sub_view_key1_default: 0,
            sub_view_key2_default: 0,
            material_parameters_size: 0,
            num_material_parameters: 0,
            material_parameters: null_mut(),
            material_default_parameters_size: 0,
            material_default_parameters: null_mut(),
            num_nodes: 0,
            nodes: null_mut(),
            num_scalar_parameters: 0,
            scalar_parameters: null_mut(),
            num_texture_parameters: 0,
            texture_parameters: null_mut(),
        }
    }
}

fn physis_get_shader_parameter_array(
    parameter_array: &Vec<ResourceParameter>,
) -> (u32, *mut physis_ShaderParameter) {
    let mut vec = vec![];

    for resource in parameter_array {
        vec.push(physis_ShaderParameter {
            name: ffi_to_c_string(&resource.name),
            slot: resource.slot,
        })
    }

    let result = (vec.len() as u32, vec.as_mut_ptr());

    mem::forget(vec);

    result
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_shpk_parse(platform: Platform, buffer: physis_Buffer) -> physis_SHPK {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(shpk) = ShaderPackage::from_existing(platform, data) {
        let mut c_vertex_shaders = vec![];
        let mut c_fragment_shaders = vec![];

        for shader in &shpk.vertex_shaders {
            let mut bytecode = shader.bytecode.clone();

            let (num_scalar_params, scalar_params) =
                physis_get_shader_parameter_array(&shader.scalar_parameters);
            let (num_resource_params, resource_params) =
                physis_get_shader_parameter_array(&shader.resource_parameters);

            let shader = physis_Shader {
                len: bytecode.len() as u32,
                bytecode: bytecode.as_mut_ptr(),
                num_scalar_parameters: num_scalar_params,
                scalar_parameters: scalar_params,
                num_resource_parameters: num_resource_params,
                resource_parameters: resource_params,
            };

            c_vertex_shaders.push(shader);

            mem::forget(bytecode);
        }

        for shader in &shpk.pixel_shaders {
            let mut bytecode = shader.bytecode.clone();

            let (num_scalar_params, scalar_params) =
                physis_get_shader_parameter_array(&shader.scalar_parameters);
            let (num_resource_params, resource_params) =
                physis_get_shader_parameter_array(&shader.resource_parameters);

            let shader = physis_Shader {
                len: bytecode.len() as u32,
                bytecode: bytecode.as_mut_ptr(),
                num_scalar_parameters: num_scalar_params,
                scalar_parameters: scalar_params,
                num_resource_parameters: num_resource_params,
                resource_parameters: resource_params,
            };

            c_fragment_shaders.push(shader);

            mem::forget(bytecode);
        }

        let mut system_keys = shpk.system_keys.clone();
        let mut scene_keys = shpk.scene_keys.clone();
        let mut material_keys = shpk.material_keys.clone();
        let mut material_params = shpk.material_parameters.clone();
        let mut material_default_params = shpk.mat_param_defaults.clone();

        let mut nodes = Vec::new();
        for node in &shpk.nodes {
            nodes.push(convert_node(node));
        }

        let (num_scalar_params, scalar_params) =
            physis_get_shader_parameter_array(&shpk.scalar_parameters);
        let (num_texture_params, texture_params) =
            physis_get_shader_parameter_array(&shpk.texture_parameters);

        let mat = physis_SHPK {
            num_vertex_shaders: c_vertex_shaders.len() as u32,
            vertex_shaders: c_vertex_shaders.as_mut_ptr(),
            num_pixel_shaders: c_fragment_shaders.len() as u32,
            pixel_shaders: c_fragment_shaders.as_mut_ptr(),
            num_system_keys: system_keys.len() as u32,
            system_keys: system_keys.as_mut_ptr(),
            num_scene_keys: scene_keys.len() as u32,
            scene_keys: scene_keys.as_mut_ptr(),
            num_material_keys: material_keys.len() as u32,
            material_keys: material_keys.as_mut_ptr(),
            sub_view_key1_default: shpk.sub_view_key1_default,
            sub_view_key2_default: shpk.sub_view_key2_default,
            material_parameters_size: shpk.material_parameters_size,
            num_material_parameters: material_params.len() as u32,
            material_parameters: material_params.as_mut_ptr(),
            material_default_parameters_size: material_default_params.len() as u32,
            material_default_parameters: material_default_params.as_mut_ptr(),
            num_nodes: nodes.len() as u32,
            nodes: nodes.as_mut_ptr(),
            num_scalar_parameters: num_scalar_params,
            scalar_parameters: scalar_params,
            num_texture_parameters: num_texture_params,
            texture_parameters: texture_params,
            p_ptr: Box::leak(Box::new(shpk)),
        };

        mem::forget(c_vertex_shaders);
        mem::forget(c_fragment_shaders);
        mem::forget(system_keys);
        mem::forget(scene_keys);
        mem::forget(material_keys);
        mem::forget(material_params);
        mem::forget(material_default_params);
        mem::forget(nodes);

        mat
    } else {
        physis_SHPK::default()
    }
}

#[repr(C)]
pub struct physis_SHPKNode {
    selector: u32,
    pass_count: u32,
    pass_indices: [u8; 16],
    system_key_count: u32,
    system_keys: *mut u32,
    scene_key_count: u32,
    scene_keys: *mut u32,
    material_key_count: u32,
    material_keys: *mut u32,
    subview_key_count: u32,
    subview_keys: *mut u32,
    passes: *mut Pass,
}

fn convert_node(node: &Node) -> physis_SHPKNode {
    let mut c_system_keys = node.system_keys.clone();
    let mut c_scene_keys = node.scene_keys.clone();
    let mut c_material_keys = node.material_keys.clone();
    let mut c_subview_keys = node.subview_keys.clone();
    let mut c_passes = node.passes.clone();

    let new_node = physis_SHPKNode {
        selector: node.selector,
        pass_count: node.pass_count,
        pass_indices: node.pass_indices,
        system_key_count: node.system_keys.len() as u32,
        system_keys: c_system_keys.as_mut_ptr(),
        scene_key_count: node.scene_keys.len() as u32,
        scene_keys: c_scene_keys.as_mut_ptr(),
        material_key_count: node.material_keys.len() as u32,
        material_keys: c_material_keys.as_mut_ptr(),
        subview_key_count: node.subview_keys.len() as u32,
        subview_keys: c_subview_keys.as_mut_ptr(),
        passes: c_passes.as_mut_ptr(),
    };

    mem::forget(c_system_keys);
    mem::forget(c_scene_keys);
    mem::forget(c_material_keys);
    mem::forget(c_subview_keys);
    mem::forget(c_passes);

    new_node
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_shpk_get_node(shpk: *const physis_SHPK, key: u32) -> physis_SHPKNode {
    unsafe {
        if let Some(node) = (*(*shpk).p_ptr).find_node(key) {
            convert_node(node)
        } else {
            physis_SHPKNode {
                selector: 0,
                pass_count: 0,
                pass_indices: [0; 16],
                system_key_count: 0,
                system_keys: null_mut(),
                scene_key_count: 0,
                scene_keys: null_mut(),
                material_key_count: 0,
                material_keys: null_mut(),
                subview_key_count: 0,
                subview_keys: null_mut(),
                passes: null_mut(),
            }
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_shpk_build_selector_from_all_keys(
    system_keys: *const u32,
    system_key_count: u32,
    scene_keys: *const u32,
    scene_key_count: u32,
    material_keys: *const u32,
    material_key_count: u32,
    subview_keys: *const u32,
    subview_key_count: u32,
) -> u32 {
    let system_keys = if !system_keys.is_null() {
        unsafe { slice::from_raw_parts(system_keys, system_key_count as usize) }
    } else {
        &[]
    };
    let scene_keys = if !scene_keys.is_null() {
        unsafe { slice::from_raw_parts(scene_keys, scene_key_count as usize) }
    } else {
        &[]
    };
    let material_keys = if !material_keys.is_null() {
        unsafe { slice::from_raw_parts(material_keys, material_key_count as usize) }
    } else {
        &[]
    };
    let subview_keys = if !subview_keys.is_null() {
        unsafe { slice::from_raw_parts(subview_keys, subview_key_count as usize) }
    } else {
        &[]
    };

    ShaderPackage::build_selector_from_all_keys(
        system_keys,
        scene_keys,
        material_keys,
        subview_keys,
    )
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_shpk_crc(name: *const c_char) -> u32 {
    ShaderPackage::crc(&ffi_from_c_string(name).unwrap())
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_shpk_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(shpk) = ShaderPackage::from_existing(platform, data) {
        ffi_to_c_string(&format!("{shpk:#?}"))
    } else {
        null()
    }
}
