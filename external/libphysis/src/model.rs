// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use physis::Platform;
use physis::model::NewShapeValue;
use std::os::raw::c_char;
use std::ptr::slice_from_raw_parts;
use std::ptr::{null, null_mut};
use std::{mem, slice};

use crate::{ffi_free_string, ffi_to_c_string, ffi_to_vec, physis_Buffer};
use physis::model::vertex_declarations::VertexElement;
use physis::model::vertex_declarations::VertexType;
use physis::model::vertex_declarations::get_vertex_type_size;
use physis::model::{MDL, SubMesh, Vertex};
use physis::{ReadableFile, WritableFile};

#[repr(C)]
pub struct physis_Part {
    num_vertices: u32,
    vertices: *mut Vertex,

    streams: *mut *const u8,
    stream_sizes: *mut usize,
    stream_strides: *mut usize,
    num_streams: usize,

    num_indices: u32,
    indices: *mut u16,

    material_index: u16,

    num_submeshes: u32,
    submeshes: *mut SubMesh,

    num_shapes: u32,
    shapes: *mut physis_Shape,
}

#[repr(C)]
pub struct physis_Shape {
    name: *const c_char,
    morphed_vertices: *mut Vertex,
}

#[repr(C)]
pub struct physis_LOD {
    num_vertex_elements: u32,
    vertex_elements: *mut VertexElement,
    num_parts: u32,
    parts: *mut physis_Part,
}

#[repr(C)]
pub struct physis_MDL {
    p_ptr: *mut MDL,
    num_lod: u32,
    lods: *mut physis_LOD,
    num_affected_bones: u32,
    affected_bone_names: *mut *const c_char,
    num_material_names: u32,
    material_names: *mut *const c_char,
}

impl Default for physis_MDL {
    fn default() -> Self {
        Self {
            p_ptr: null_mut(),
            num_lod: 0,
            lods: null_mut(),
            num_affected_bones: 0,
            affected_bone_names: null_mut(),
            num_material_names: 0,
            material_names: null_mut(),
        }
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_mdl_parse(platform: Platform, buffer: physis_Buffer) -> physis_MDL {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    let Some(mdl_d) = MDL::from_existing(platform, data) else {
        return physis_MDL::default();
    };

    let mdl = Box::new(mdl_d);

    let mut c_lods: Vec<physis_LOD> = physis_mdl_update_vertices(&mdl);

    let mut c_bone_names = vec![];

    for bone_name in &mdl.affected_bone_names {
        c_bone_names.push(ffi_to_c_string(bone_name));
    }

    let mut c_material_names = vec![];

    for bone_name in &mdl.material_names {
        c_material_names.push(ffi_to_c_string(bone_name));
    }

    let mdl = physis_MDL {
        p_ptr: Box::leak(mdl),
        num_lod: c_lods.len() as u32,
        lods: c_lods.as_mut_ptr(),
        num_affected_bones: c_bone_names.len() as u32,
        affected_bone_names: c_bone_names.as_mut_ptr(),
        num_material_names: c_material_names.len() as u32,
        material_names: c_material_names.as_mut_ptr(),
    };

    mem::forget(c_bone_names);
    mem::forget(c_material_names);
    mem::forget(c_lods);

    mdl
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_mdl_write(mdl: &physis_MDL) -> physis_Buffer {
    unsafe {
        let mut buffer = (*mdl.p_ptr).write_to_buffer(Platform::Win32).unwrap();

        let leak_buffer = physis_Buffer {
            size: buffer.len() as u32,
            data: buffer.as_mut_ptr(),
        };

        mem::forget(buffer);

        leak_buffer
    }
}

fn physis_mdl_update_vertices(mdl: &MDL) -> Vec<physis_LOD> {
    let mut c_lods: Vec<physis_LOD> = Vec::new();

    for (i, lod) in mdl.lods.iter().enumerate() {
        let mut c_decls: Vec<VertexElement> = mdl.model_data.header.vertex_declarations[i]
            .elements
            .clone();

        let mut c_parts: Vec<physis_Part> = Vec::new();

        for part in &lod.parts {
            let mut c_vertices: Vec<Vertex> = part.vertices.clone();
            let mut c_indices: Vec<u16> = part.indices.clone();
            let mut c_submeshes: Vec<SubMesh> = part.submeshes.clone();
            let mut c_shapes = vec![];
            let mut c_streams = vec![];
            let mut c_stream_sizes = vec![];
            let mut c_stream_strides = part.vertex_stream_strides.clone();

            for shape in &part.shapes {
                let mut c_morphed_vertices = shape.morphed_vertices.clone();

                c_shapes.push(physis_Shape {
                    name: ffi_to_c_string(&shape.name),
                    morphed_vertices: c_morphed_vertices.as_mut_ptr(),
                });

                mem::forget(c_morphed_vertices);
            }

            for stream in &part.vertex_streams {
                let c_stream = stream.clone();
                c_streams.push(c_stream.as_ptr());
                c_stream_sizes.push(c_stream.len());

                mem::forget(c_stream);
            }

            c_parts.push(physis_Part {
                num_vertices: c_vertices.len() as u32,
                vertices: c_vertices.as_mut_ptr(),
                streams: c_streams.as_mut_ptr(),
                stream_sizes: c_stream_sizes.as_mut_ptr(),
                stream_strides: c_stream_strides.as_mut_ptr(),
                num_streams: c_stream_strides.len(),
                num_indices: c_indices.len() as u32,
                indices: c_indices.as_mut_ptr(),
                material_index: part.material_index,
                num_submeshes: c_submeshes.len() as u32,
                submeshes: c_submeshes.as_mut_ptr(),
                num_shapes: c_shapes.len() as u32,
                shapes: c_shapes.as_mut_ptr(),
            });

            mem::forget(c_vertices);
            mem::forget(c_indices);
            mem::forget(c_submeshes);
            mem::forget(c_shapes);
            mem::forget(c_streams);
            mem::forget(c_stream_sizes);
            mem::forget(c_stream_strides);
        }

        c_lods.push(physis_LOD {
            num_vertex_elements: c_decls.len() as u32,
            vertex_elements: c_decls.as_mut_ptr(),
            num_parts: c_parts.len() as u32,
            parts: c_parts.as_mut_ptr(),
        });

        mem::forget(c_decls);
        mem::forget(c_parts);
    }

    c_lods
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_mdl_replace_vertices(
    mdl: *mut physis_MDL,
    lod_index: u32,
    part_index: u32,
    num_vertices: u32,
    vertices_ptr: *const Vertex,
    num_indices: u32,
    indices_ptr: *const u16,
    num_submeshes: u32,
    submeshes_ptr: *const SubMesh,
) {
    unsafe {
        (*(*mdl).p_ptr).replace_vertices(
            lod_index as usize,
            part_index as usize,
            &*std::ptr::slice_from_raw_parts(vertices_ptr, num_vertices as usize),
            &*std::ptr::slice_from_raw_parts(indices_ptr, num_indices as usize),
            &*std::ptr::slice_from_raw_parts(submeshes_ptr, num_submeshes as usize),
        );

        // We need to update the C version of these LODs as well
        let mut new_lods = physis_mdl_update_vertices((*mdl).p_ptr.as_ref().unwrap());

        (*mdl).lods = new_lods.as_mut_ptr();

        mem::forget(new_lods);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_mdl_remove_shape_meshes(mdl: *mut physis_MDL) {
    unsafe {
        (*(*mdl).p_ptr).remove_shape_meshes();
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_mdl_add_shape_mesh(
    mdl: *mut physis_MDL,
    lod_index: u32,
    shape_index: u32,
    shape_mesh_index: u32,
    part_index: u32,
    num_shape_values: u32,
    shape_values: *const NewShapeValue,
) {
    unsafe {
        (*(*mdl).p_ptr).add_shape_mesh(
            lod_index as usize,
            shape_index as usize,
            shape_mesh_index as usize,
            part_index as usize,
            &*slice_from_raw_parts(shape_values, num_shape_values as usize),
        );

        // We need to update the C version of these LODs as well
        let mut new_lods = physis_mdl_update_vertices((*mdl).p_ptr.as_ref().unwrap());

        (*mdl).lods = new_lods.as_mut_ptr();

        mem::forget(new_lods);
    }
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_get_vertex_type_size(vertex_type: VertexType) -> usize {
    get_vertex_type_size(vertex_type)
}

#[unsafe(no_mangle)]
pub extern "C" fn physis_mdl_free(mdl: &physis_MDL) {
    unsafe {
        let lods = ffi_to_vec(mdl.lods, mdl.num_lod);
        for lod in &lods {
            let parts = ffi_to_vec(lod.parts, lod.num_parts);
            for part in &parts {
                let vertices = ffi_to_vec(part.vertices, part.num_vertices);
                drop(vertices);

                let streams = ffi_to_vec(part.streams, part.num_streams as u32);
                drop(streams);

                let stream_sizes = ffi_to_vec(part.stream_sizes, part.num_streams as u32);
                drop(stream_sizes);

                let stream_strides = ffi_to_vec(part.stream_strides, part.num_streams as u32);
                drop(stream_strides);

                let indices = ffi_to_vec(part.indices, part.num_indices);
                drop(indices);

                let submeshes = ffi_to_vec(part.submeshes, part.num_submeshes);
                drop(submeshes);

                let shapes = ffi_to_vec(part.shapes, part.num_shapes);
                for shape in &shapes {
                    ffi_free_string(shape.name);
                    let morphed_vertices = ffi_to_vec(shape.morphed_vertices, part.num_vertices);
                    drop(morphed_vertices);
                }
                drop(shapes);
            }
            drop(parts);

            let vertex_elements = ffi_to_vec(lod.vertex_elements, lod.num_vertex_elements);
            drop(vertex_elements);
        }
        drop(lods);

        let affected_bone_names = ffi_to_vec(mdl.affected_bone_names, mdl.num_affected_bones);
        for name in &affected_bone_names {
            ffi_free_string(*name);
        }
        drop(affected_bone_names);

        let material_names = ffi_to_vec(mdl.material_names, mdl.num_material_names);
        for name in &material_names {
            ffi_free_string(*name);
        }
        drop(material_names);

        drop(Box::from_raw(mdl.p_ptr));
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn physis_mdl_debug(
    platform: Platform,
    buffer: physis_Buffer,
) -> *const c_char {
    let data = unsafe { slice::from_raw_parts(buffer.data, buffer.size as usize) };

    if let Some(mdl) = MDL::from_existing(platform, data) {
        ffi_to_c_string(&format!("{mdl:#?}"))
    } else {
        null()
    }
}
