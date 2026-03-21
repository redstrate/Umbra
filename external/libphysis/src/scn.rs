// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use crate::ffi_to_c_string;
use crate::layer::{physis_Layer, to_c_layer};
use crate::tmb::{physis_Tmb, to_c_tmb};
use physis::scn::ScnSGActionControllerDescriptor;
use physis::scn::{ScnLayerGroup, ScnSection, ScnTimeline, ScnTimelineInstance};
use std::ffi::c_char;

#[repr(C)]
pub struct physis_ScnSection {
    num_layer_groups: u32,
    layer_groups: *const physis_ScnLayerGroup,

    general: physis_ScnGeneralSection,
    timelines: physis_ScnTimelinesSection,

    num_lgb_paths: u32,
    lgb_paths: *const *const c_char,

    action_descriptors: physis_ScnSGActionDescriptors,
}

#[repr(C)]
pub struct physis_ScnGeneralSection {
    bg_path: *const c_char,
}

#[repr(C)]
pub struct physis_ScnTimelinesSection {
    timeline_count: u32,
    timelines: *const physis_ScnTimeline,
}

#[repr(C)]
pub struct physis_ScnTimeline {
    sub_id: u32,
    tmb: physis_Tmb,
    instance_count: u32,
    instances: *const ScnTimelineInstance,
}

#[repr(C)]
pub struct physis_ScnLayerGroup {
    layer_group_id: u32,
    name: *const c_char,
    layer_count: u32,
    layers: *mut physis_Layer,
}

#[repr(C)]
pub struct physis_ScnSGActionDescriptors {
    descriptor_count: u32,
    descriptors: *const ScnSGActionControllerDescriptor,
}

pub fn to_c_section(section: &ScnSection) -> physis_ScnSection {
    let mut c_layer_groups = Vec::new();
    for layer_group in &section.layer_groups {
        c_layer_groups.push(to_c_layer_group(layer_group));
    }

    let mut c_lgb_paths = Vec::new();
    for lgb_path in &section.lgb_paths {
        c_lgb_paths.push(ffi_to_c_string(lgb_path));
    }

    let general = physis_ScnGeneralSection {
        bg_path: ffi_to_c_string(&section.general.bg_path.value),
    };

    let mut c_timelines = Vec::new();
    for timeline in &section.timelines.timelines {
        c_timelines.push(to_c_timeline(timeline));
    }

    let timelines = physis_ScnTimelinesSection {
        timeline_count: c_timelines.len() as u32,
        timelines: c_timelines.as_ptr(),
    };

    let c_descriptors = section.action_descriptors.descriptors.clone();

    let action_descriptors = physis_ScnSGActionDescriptors {
        descriptor_count: c_descriptors.len() as u32,
        descriptors: c_descriptors.as_ptr(),
    };

    let scn = physis_ScnSection {
        num_layer_groups: c_layer_groups.len() as u32,
        layer_groups: c_layer_groups.as_ptr(),
        general,
        timelines,
        num_lgb_paths: c_lgb_paths.len() as u32,
        lgb_paths: c_lgb_paths.as_ptr(),
        action_descriptors,
    };

    std::mem::forget(c_layer_groups);
    std::mem::forget(c_lgb_paths);
    std::mem::forget(c_timelines);
    std::mem::forget(c_descriptors);

    scn
}

pub fn to_c_layer_group(section: &ScnLayerGroup) -> physis_ScnLayerGroup {
    let mut c_layers = Vec::new();

    for layer in &section.layers {
        c_layers.push(to_c_layer(layer));
    }

    let layer_group = physis_ScnLayerGroup {
        layer_group_id: section.layer_group_id,
        name: ffi_to_c_string(&section.name.value),
        layer_count: c_layers.len() as u32,
        layers: c_layers.as_mut_ptr(),
    };

    std::mem::forget(c_layers);

    layer_group
}

pub fn to_c_timeline(timeline: &ScnTimeline) -> physis_ScnTimeline {
    let c_instances = timeline.instances.clone();

    let c_timeline = physis_ScnTimeline {
        sub_id: timeline.sub_id,
        tmb: to_c_tmb(&timeline.tmb),
        instance_count: c_instances.len() as u32,
        instances: c_instances.as_ptr(),
    };

    std::mem::forget(c_instances);

    c_timeline
}
