/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkSampler.h"

#include "GrTextureAccess.h"
#include "GrVkGpu.h"

static inline VkSamplerAddressMode tile_to_vk_sampler_address(SkShader::TileMode tm) {
    static const VkSamplerAddressMode gWrapModes[] = {
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT
    };
    GR_STATIC_ASSERT(SkShader::kTileModeCount == SK_ARRAY_COUNT(gWrapModes));
    GR_STATIC_ASSERT(0 == SkShader::kClamp_TileMode);
    GR_STATIC_ASSERT(1 == SkShader::kRepeat_TileMode);
    GR_STATIC_ASSERT(2 == SkShader::kMirror_TileMode);
    return gWrapModes[tm];
}

GrVkSampler* GrVkSampler::Create(const GrVkGpu* gpu, const GrTextureAccess& textureAccess) {

    static VkFilter vkMinFilterModes[] = {
        VK_FILTER_NEAREST,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR
    };
    static VkFilter vkMagFilterModes[] = {
        VK_FILTER_NEAREST,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR
    };

    const GrTextureParams& params = textureAccess.getParams();

    VkSamplerCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkSamplerCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.pNext = 0;
    createInfo.flags = 0;
    createInfo.magFilter = vkMagFilterModes[params.filterMode()];
    createInfo.minFilter = vkMinFilterModes[params.filterMode()];
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    createInfo.addressModeU = tile_to_vk_sampler_address(params.getTileModeX());
    createInfo.addressModeV = tile_to_vk_sampler_address(params.getTileModeY());
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // Shouldn't matter
    createInfo.mipLodBias = 0.0f;
    createInfo.anisotropyEnable = VK_FALSE;
    createInfo.maxAnisotropy = 1.0f;
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_NEVER;
    createInfo.minLod = 0.0f;
    createInfo.maxLod = 0.0f;
    createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;

    VkSampler sampler;
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), CreateSampler(gpu->device(),
                                                          &createInfo,
                                                          nullptr,
                                                          &sampler));

    return new GrVkSampler(sampler);
}

void GrVkSampler::freeGPUData(const GrVkGpu* gpu) const {
    SkASSERT(fSampler);
    GR_VK_CALL(gpu->vkInterface(), DestroySampler(gpu->device(), fSampler, nullptr));
}