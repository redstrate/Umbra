// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

use std::os::raw::c_char;

use crate::ffi_to_c_string;
use physis::layer::LayerEntryData::*;
use physis::layer::*;

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_BGInstanceObject {
    asset_path: *const c_char,
    collision_asset_path: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_LightInstanceObject {
    light_type: LightType,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_VfxInstanceObject {
    asset_path: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_GameInstanceObject {
    base_id: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_EventInstanceObject {
    parent_data: physis_GameInstanceObject,
    bound_instance_id: u32,
    linked_instance_id: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_PopRangeInstanceObject {
    pop_type: PopType,
    index: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_NPCInstanceObject {
    parent_data: physis_GameInstanceObject,
    pop_weather: u32,
    pop_time_start: u8,
    pop_time_end: u8,
    move_ai: u32,
    wandering_range: u8,
    route: u8,
    event_group: u16,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_ENPCInstanceObject {
    parent_data: physis_NPCInstanceObject,
    behavior: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_TriggerBoxInstanceObject {
    trigger_box_shape: TriggerBoxShape,
    priority: i16,
    enabled: bool,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_MapRangeInstanceObject {
    parent_data: physis_TriggerBoxInstanceObject,
    place_name_block: u32,
    place_name_spot: u32,
    rest_bonus_effective: bool,
    discovery_id: u8,
    place_name_enabled: bool,
    discovery_enabled: bool,
    rest_bonus_enabled: bool,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_SharedGroupInstanceObject {
    asset_path: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_AetheryteInstanceObject {
    parent_data: physis_GameInstanceObject,
    bound_instance_id: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_ExitRangeInstanceObject {
    parent_data: physis_TriggerBoxInstanceObject,
    exit_type: ExitType,
    zone_id: u16,
    territory_type: u16,
    destination_instance_id: u32,
    return_instance_id: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_EventRangeInstanceObject {
    parent_data: physis_TriggerBoxInstanceObject,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_ChairMarkerInstanceObject {}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_PrefetchRangeInstanceObject {
    parent_data: physis_TriggerBoxInstanceObject,
    bound_instance_id: u32,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_EnvSetInstanceObject {
    asset_path: *const c_char,
    bound_instance_id: u32,
    shape: EnvSetShape,
    is_env_map_shooting_point: bool,
    priority: u8,
    effective_range: f32,
    interpolation_time: i32,
    reverb: f32,
    filter: f32,
    sound_asset_path: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_EnvLocationObject {
    ambient_light_asset_path: *const c_char,
    env_map_asset_path: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_SoundInstanceObject {
    sound_effect_param: i32,
    asset_path: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_CollisionBoxInstanceObject {
    parent_data: physis_TriggerBoxInstanceObject,
    collision_asset_path: *const c_char,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_DoorRangeInstanceObject {
    parent_data: physis_TriggerBoxInstanceObject,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_LineVFXInstanceObject {
    line_style: LineStyle,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_TreasureInstanceObject {
    nonpop_init_zone: u8,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_TargetMarkerInstanceObject {
    nameplate_offset_y: f32,
    target_market_type: TargetMarkerType,
}

#[repr(C)]
#[derive(Clone, Copy)]
#[allow(dead_code)]
pub enum physis_LayerEntry {
    None, // NOTE: a thing until every layer entry is supported
    BG(physis_BGInstanceObject),
    LayLight(physis_LightInstanceObject),
    Vfx(physis_VfxInstanceObject),
    EventObject(physis_EventInstanceObject),
    PopRange(physis_PopRangeInstanceObject),
    EventNPC(physis_ENPCInstanceObject),
    MapRange(physis_MapRangeInstanceObject),
    SharedGroup(physis_SharedGroupInstanceObject),
    Aetheryte(physis_AetheryteInstanceObject),
    ExitRange(physis_ExitRangeInstanceObject),
    EventRange(physis_EventRangeInstanceObject),
    ChairMarker(physis_ChairMarkerInstanceObject),
    PrefetchRange(physis_PrefetchRangeInstanceObject),
    EnvSet(physis_EnvSetInstanceObject),
    EnvLocation(physis_EnvLocationObject),
    Sound(physis_SoundInstanceObject),
    CollisionBox(physis_CollisionBoxInstanceObject),
    DoorRange(physis_DoorRangeInstanceObject),
    LineVFX(physis_LineVFXInstanceObject),
    Treasure(physis_TreasureInstanceObject),
    TargetMarker(physis_TargetMarkerInstanceObject),
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_InstanceObject {
    pub instance_id: u32,
    pub name: *const c_char,
    pub transform: Transformation,
    pub data: physis_LayerEntry,
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct physis_Layer {
    pub objects: *mut physis_InstanceObject,
    pub num_objects: u32,
    pub name: *const c_char,
    pub id: u32,
    pub festival_id: u16,
    pub festival_phase_id: u16,
}

fn convert_gameinstanceobject(obj: &GameInstanceObject) -> physis_GameInstanceObject {
    physis_GameInstanceObject {
        base_id: obj.base_id,
    }
}

fn convert_triggerboxinstanceobject(
    obj: &TriggerBoxInstanceObject,
) -> physis_TriggerBoxInstanceObject {
    physis_TriggerBoxInstanceObject {
        trigger_box_shape: obj.trigger_box_shape,
        priority: obj.priority,
        enabled: obj.enabled,
    }
}

pub(crate) fn convert_data(data: &LayerEntryData) -> physis_LayerEntry {
    match data {
        BG(bg) => physis_LayerEntry::BG(physis_BGInstanceObject {
            asset_path: ffi_to_c_string(&bg.asset_path.value),
            collision_asset_path: ffi_to_c_string(&bg.collision_asset_path.value),
        }),
        LayLight(light) => physis_LayerEntry::LayLight(physis_LightInstanceObject {
            light_type: light.light_type,
        }),
        Vfx(vfx) => physis_LayerEntry::Vfx(physis_VfxInstanceObject {
            asset_path: ffi_to_c_string(&vfx.asset_path.value),
        }),
        EventObject(eobj) => physis_LayerEntry::EventObject(physis_EventInstanceObject {
            parent_data: convert_gameinstanceobject(&eobj.parent_data),
            bound_instance_id: eobj.bound_instance_id,
            linked_instance_id: eobj.linked_instance_id,
        }),
        PopRange(pop) => physis_LayerEntry::PopRange(physis_PopRangeInstanceObject {
            pop_type: pop.pop_type,
            index: pop.index,
        }),
        EventNPC(enpc) => physis_LayerEntry::EventNPC(physis_ENPCInstanceObject {
            parent_data: physis_NPCInstanceObject {
                parent_data: convert_gameinstanceobject(&enpc.parent_data.parent_data),
                pop_weather: enpc.parent_data.pop_weather,
                pop_time_start: enpc.parent_data.pop_time_start,
                pop_time_end: enpc.parent_data.pop_time_end,
                move_ai: enpc.parent_data.move_ai,
                wandering_range: enpc.parent_data.wandering_range,
                route: enpc.parent_data.route,
                event_group: enpc.parent_data.event_group,
            },
            behavior: enpc.behavior,
        }),
        MapRange(map_range) => physis_LayerEntry::MapRange(physis_MapRangeInstanceObject {
            parent_data: convert_triggerboxinstanceobject(&map_range.parent_data),
            place_name_block: map_range.place_name_block,
            place_name_spot: map_range.place_name_spot,
            rest_bonus_effective: map_range.rest_bonus_effective,
            discovery_id: map_range.discovery_id,
            place_name_enabled: map_range.place_name_enabled,
            discovery_enabled: map_range.discovery_enabled,
            rest_bonus_enabled: map_range.rest_bonus_enabled,
        }),
        SharedGroup(shared_group) => {
            physis_LayerEntry::SharedGroup(physis_SharedGroupInstanceObject {
                asset_path: ffi_to_c_string(&shared_group.asset_path.value),
            })
        }
        Aetheryte(aetheryte) => physis_LayerEntry::Aetheryte(physis_AetheryteInstanceObject {
            parent_data: convert_gameinstanceobject(&aetheryte.parent_data),
            bound_instance_id: aetheryte.bound_instance_id,
        }),
        ExitRange(exit_range) => physis_LayerEntry::ExitRange(physis_ExitRangeInstanceObject {
            parent_data: convert_triggerboxinstanceobject(&exit_range.parent_data),
            exit_type: exit_range.exit_type,
            zone_id: exit_range.zone_id,
            territory_type: exit_range.territory_type,
            destination_instance_id: exit_range.destination_instance_id,
            return_instance_id: exit_range.return_instance_id,
        }),
        EventRange(event_range) => physis_LayerEntry::EventRange(physis_EventRangeInstanceObject {
            parent_data: convert_triggerboxinstanceobject(&event_range.parent_data),
        }),
        ChairMarker(_) => physis_LayerEntry::ChairMarker(physis_ChairMarkerInstanceObject {}),
        PrefetchRange(prefetch_range) => {
            physis_LayerEntry::PrefetchRange(physis_PrefetchRangeInstanceObject {
                parent_data: convert_triggerboxinstanceobject(&prefetch_range.parent_data),
                bound_instance_id: prefetch_range.bound_instance_id,
            })
        }
        EnvSet(env_set) => physis_LayerEntry::EnvSet(physis_EnvSetInstanceObject {
            asset_path: ffi_to_c_string(&env_set.asset_path.value),
            bound_instance_id: env_set.bound_instance_id,
            shape: env_set.shape,
            is_env_map_shooting_point: env_set.is_env_map_shooting_point,
            priority: env_set.priority,
            effective_range: env_set.effective_range,
            interpolation_time: env_set.interpolation_time,
            reverb: env_set.reverb,
            filter: env_set.filter,
            sound_asset_path: ffi_to_c_string(&env_set.sound_asset_path.value),
        }),
        EnvLocation(env_location) => physis_LayerEntry::EnvLocation(physis_EnvLocationObject {
            ambient_light_asset_path: ffi_to_c_string(&env_location.ambient_light_asset_path.value),
            env_map_asset_path: ffi_to_c_string(&env_location.env_map_asset_path.value),
        }),
        Sound(sound) => physis_LayerEntry::Sound(physis_SoundInstanceObject {
            sound_effect_param: sound.sound_effect_param,
            asset_path: ffi_to_c_string(&sound.asset_path.value),
        }),
        CollisionBox(collision_box) => {
            physis_LayerEntry::CollisionBox(physis_CollisionBoxInstanceObject {
                parent_data: convert_triggerboxinstanceobject(&collision_box.parent_data),
                collision_asset_path: ffi_to_c_string(&collision_box.collision_asset_path.value),
            })
        }
        DoorRange(door_range) => physis_LayerEntry::DoorRange(physis_DoorRangeInstanceObject {
            parent_data: convert_triggerboxinstanceobject(&door_range.parent_data),
        }),
        LineVFX(line_vfx) => physis_LayerEntry::LineVFX(physis_LineVFXInstanceObject {
            line_style: line_vfx.line_style,
        }),
        Treasure(treasure) => physis_LayerEntry::Treasure(physis_TreasureInstanceObject {
            nonpop_init_zone: treasure.nonpop_init_zone,
        }),
        TargetMarker(target_marker) => {
            physis_LayerEntry::TargetMarker(physis_TargetMarkerInstanceObject {
                nameplate_offset_y: target_marker.nameplate_offset_y,
                target_market_type: target_marker.target_marker_type,
            })
        }
        _ => physis_LayerEntry::None,
    }
}

pub(crate) fn to_c_layer(layer: &Layer) -> physis_Layer {
    let mut c_objects = vec![];

    for object in &layer.objects {
        c_objects.push(physis_InstanceObject {
            instance_id: object.instance_id,
            name: ffi_to_c_string(&object.name.value),
            transform: object.transform,
            data: convert_data(&object.data),
        });
    }

    let layer = physis_Layer {
        objects: c_objects.as_mut_ptr(),
        num_objects: c_objects.len() as u32,
        name: ffi_to_c_string(&layer.header.name.value),
        id: layer.header.layer_id,
        festival_id: layer.header.festival_id,
        festival_phase_id: layer.header.festival_phase_id,
    };

    std::mem::forget(c_objects);

    layer
}
