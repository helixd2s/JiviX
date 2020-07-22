#include "./vlr/Framebuffer.hpp"
#include "./vlr/Driver.hpp"
#include "./vlr/PipelineLayout.hpp"

namespace vlr {

    // 
    void Framebuffer::constructor(vkt::uni_ptr<Driver> driver) {
        this->driver = driver;
    };

    // 
    void Framebuffer::createRenderPass() {
        auto device = this->driver->getDeviceDispatch();

        //
        auto colAttachment = vkh::VkAttachmentDescription{
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_GENERAL,
            .finalLayout = VK_IMAGE_LAYOUT_GENERAL,
        };

        //
        auto depAttachment = vkh::VkAttachmentDescription{
            .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        //
        auto dep0 = vkh::VkSubpassDependency{ .srcSubpass = VK_SUBPASS_EXTERNAL, .dstSubpass = 0u };
        auto dep1 = vkh::VkSubpassDependency{ .srcSubpass = 0u, .dstSubpass = VK_SUBPASS_EXTERNAL };

        { // Fixing Linux Issue?
            auto srcStageMask = vkh::VkPipelineStageFlags{.eColorAttachmentOutput = 1, .eTransfer = 1, .eBottomOfPipe = 1}; ASSIGN(dep0, srcStageMask);
            auto dstStageMask = vkh::VkPipelineStageFlags{.eColorAttachmentOutput = 1};                                     ASSIGN(dep0, dstStageMask);
            auto srcAccessMask = vkh::VkAccessFlags{.eColorAttachmentWrite = 1};                                            ASSIGN(dep0, srcAccessMask);
            auto dstAccessMask = vkh::VkAccessFlags{.eColorAttachmentRead = 1, .eColorAttachmentWrite = 1};                 ASSIGN(dep0, dstAccessMask);
            auto dependencyFlags = vkh::VkDependencyFlags{.eByRegion = 1};                                                  ASSIGN(dep0, dependencyFlags);
        };

        { // Fixing Linux Issue?
            auto srcStageMask = vkh::VkPipelineStageFlags{.eColorAttachmentOutput = 1};                                     ASSIGN(dep1, srcStageMask);
            auto dstStageMask = vkh::VkPipelineStageFlags{.eTopOfPipe = 1, .eColorAttachmentOutput = 1, .eTransfer = 1};    ASSIGN(dep1, dstStageMask);
            auto srcAccessMask = vkh::VkAccessFlags{.eColorAttachmentRead = 1, .eColorAttachmentWrite = 1};                 ASSIGN(dep1, srcAccessMask);
            auto dstAccessMask = vkh::VkAccessFlags{.eColorAttachmentRead = 1, .eColorAttachmentWrite = 1};                 ASSIGN(dep1, dstAccessMask);
            auto dependencyFlags = vkh::VkDependencyFlags{.eByRegion = 1};                                                  ASSIGN(dep1, dependencyFlags);
        };

        {
            vkh::VsRenderPassCreateInfoHelper rpsInfo = {};
            for (uint32_t b = 0u; b < 8u; b++) { rpsInfo.addColorAttachment(colAttachment); };
            rpsInfo.setDepthStencilAttachment(depAttachment);
            rpsInfo.addSubpassDependency(dep0);
            rpsInfo.addSubpassDependency(dep1);
            vkh::handleVk(device->CreateRenderPass(rpsInfo, nullptr, &rasterFBO.renderPass));
            vkh::handleVk(device->CreateRenderPass(rpsInfo, nullptr, &resampleFBO.renderPass));
        };

    };

    // 
    void Framebuffer::createFramebuffer(uint32_t width = 1920u, uint32_t height = 1200u) { //
        auto fbusage = vkh::VkImageUsageFlags{.eTransferDst = 1, .eSampled = 1, .eStorage = 1, .eColorAttachment = 1 };
        auto aspect = vkh::VkImageAspectFlags{.eColor = 1};
        auto apres = vkh::VkImageSubresourceRange{.aspectMask = aspect};
        auto device = this->driver->getDeviceDispatch();
        auto& allocInfo = driver->memoryAllocationInfo();

        // 
        std::vector<VkImageView> rasterAttachments = {};
        std::vector<VkImageView> resampleAttachments = {};

        {   //
            auto aspect = vkh::VkImageAspectFlags{.eDepth = 1, .eStencil = 1};
            auto dpuse = vkh::VkImageUsageFlags{.eTransferDst = 1, .eSampled = 1, .eDepthStencilAttachment = 1 };
            auto dpres = vkh::VkImageSubresourceRange{.aspectMask = aspect};
            dpuse = dpuse | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            
            //
            depthStencilImage = vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(vkh::VkImageCreateInfo{
                .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                .extent = {width,height,1u},
                .usage = dpuse,
            }, allocInfo), vkh::VkImageViewCreateInfo{
                .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
                .subresourceRange = dpres,
            }, VK_IMAGE_LAYOUT_GENERAL);
        };

        // 
        auto createImage = [=,this](VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT){
            return vkt::ImageRegion(std::make_shared<vkt::ImageAllocation>(vkh::VkImageCreateInfo{
                .format = format,
                .extent = {width,height,1u},
                .usage = fbusage,
            }, allocInfo), vkh::VkImageViewCreateInfo{
                .format = format,
                .subresourceRange = apres
            });
        };

        // 
        auto createFramebuffer = [&,this](RenderPass& fbo, const std::vector<VkImageView>& attachments){
            vkh::handleVk(device->CreateFramebuffer(vkh::VkFramebufferCreateInfo{
                .renderPass = fbo.renderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = width,
                .height = height
            }, nullptr, &fbo.framebuffer));
        };

        // 
        for (uint32_t b = 0u; b < 12u; b++) { // 
            currentsImages.push_back(createImage());
            previousImages.push_back(createImage());
        };

        // 
        for (uint32_t b = 0u; b < 8u; b++) {
            resampleImages.push_back(createImage());
            resampleAttachments.push_back(resampleImages.back());
            rasterImages.push_back(createImage());
            rasterAttachments.push_back(rasterImages.back());
        };

        // 
        {
            this->samplers.push_back(VK_NULL_HANDLE);
            this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .unnormalizedCoordinates = true,
            }, nullptr, &this->samplers.back());

            this->samplers.push_back(VK_NULL_HANDLE);
            this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            }, nullptr, &this->samplers.back());

            this->samplers.push_back(VK_NULL_HANDLE);
            this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            }, nullptr, &this->samplers.back());

            this->samplers.push_back(VK_NULL_HANDLE);
            this->driver->getDeviceDispatch()->CreateSampler(vkh::VkSamplerCreateInfo{
                .magFilter = VK_FILTER_LINEAR,
                .minFilter = VK_FILTER_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            }, nullptr, &this->samplers.back());
        };

        // 
        resampleAttachments.push_back(depthStencilImage);
        rasterAttachments.push_back(depthStencilImage);

        // 
        createFramebuffer(resampleFBO, resampleAttachments);
        createFramebuffer(rasterFBO, rasterAttachments);
    };

    // 
    void Framebuffer::createDescriptorSet(vkt::uni_ptr<PipelineLayout> pipelineLayout) {
        auto device = this->driver->getDeviceDispatch();
        auto descriptorSetInfo = vkh::VsDescriptorSetCreateInfoHelper(pipelineLayout->getBufferViewSetLayout(), pipelineLayout->getDescriptorPool());
        auto createDescriptorSetImages = [&,this](const std::vector<vkt::ImageRegion>& images, const uint32_t& binding = 0u) {
            auto& handle = descriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = binding,
                .dstArrayElement = 0u,
                .descriptorCount = uint32_t(images.size()),
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
            });
            for (uintptr_t i = 0; i < images.size(); i++) {
                handle.offset<VkDescriptorImageInfo>(i) = images[i];
            };
        };

        {   // Samplers
            auto& handle = descriptorSetInfo.pushDescription(vkh::VkDescriptorUpdateTemplateEntry{
                .dstBinding = 4u,
                .dstArrayElement = 0u,
                .descriptorCount = uint32_t(samplers.size()),
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER
            });
            for (uintptr_t i = 0; i < samplers.size(); i++) {
                handle.offset<VkDescriptorImageInfo>(i)->sampler = samplers[i];
            };
        };

        // 
        createDescriptorSetImages(currentsImages,0u);
        createDescriptorSetImages(previousImages,1u);
        createDescriptorSetImages(resampleImages,2u);
        createDescriptorSetImages(rasterImages,3u);

        // 
        vkh::handleVk(vkt::AllocateDescriptorSetWithUpdate(device, descriptorSetInfo, this->set, this->updated));
    };

};