#pragma once
#include "./Config.hpp"

namespace vlr {

    struct RenderPass {
        VkFramebuffer framebuffer = {};
        VkRenderPass renderPass = {};
        
        std::vector<vkh::VkPipelineColorBlendAttachmentState> blendStates = {};
        std::vector<vkh::VkClearValue> clearValues = {};
        vkh::VsDescriptorSetCreateInfoHelper descriptorSetInfo = {};
    };

    class Framebuffer : public std::enable_shared_from_this<Framebuffer> { protected: 
        RenderPass rasterFBO = {}, resampleFBO = {}; VkDescriptorSet set = {};
        vkt::ImageRegion depthStencilImage = {};
        std::vector<VkSampler> samplers = {};
        std::vector<vkt::ImageRegion> currentsImages = {};  // Current Frame
        std::vector<vkt::ImageRegion> previousImages = {};  // Previous Frame
        std::vector<vkt::ImageRegion> resampleImages = {}; // Resampled Previous Frame
        std::vector<vkt::ImageRegion> rasterImages = {};    // Rasterized Frame
        VkRect2D scissor = {}; VkViewport viewport = {};    // 
        uint32_t width = 1920u, height = 1200u;
        std::shared_ptr<Driver> driver = {};
        bool updated = false;

    public: 
        Framebuffer() { this->constructor(); };
        Framebuffer(vkt::uni_ptr<Driver> driver) { this->constructor(driver); };

        // 
        virtual void constructor() {};
        virtual void constructor(vkt::uni_ptr<Driver> driver);
        virtual void createRenderPass();
        virtual void createFramebuffer(uint32_t width = 1920u, uint32_t height = 1200u);
        virtual void createDescriptorSet(vkt::uni_ptr<PipelineLayout> layout);
    };

};