#pragma once
#include "./Config.hpp"
#include "./SetBase.hpp"

namespace vlr {

    class InstanceSet : public SetBase_T<vkh::VsGeometryInstance> { protected: 
        vkt::uni_ptr<Driver> driver = {};

    public: 
        InstanceSet() : SetBase_T<vkh::VsGeometryInstance>() { this->constructorExtend0(); };
        InstanceSet(vkt::uni_ptr<Driver> driver, vkt::uni_arg<DataSetCreateInfo> info) : SetBase_T<vkh::VsGeometryInstance>(driver, info) { this->constructorExtend0(driver); };

        virtual void constructorExtend0() {};
        virtual void constructorExtend0(vkt::uni_ptr<Driver> driver) {};

        // 
        virtual const vkt::VectorBase& getCpuBuffer() const override { return SetBase_T<vkh::VsGeometryInstance>::getCpuBuffer(); };
        virtual const vkt::VectorBase& getGpuBuffer() const override { return SetBase_T<vkh::VsGeometryInstance>::getGpuBuffer(); };
        virtual vkt::VectorBase& getCpuBuffer() override { return SetBase_T<vkh::VsGeometryInstance>::getCpuBuffer(); };
        virtual vkt::VectorBase& getGpuBuffer() override { return SetBase_T<vkh::VsGeometryInstance>::getGpuBuffer(); };
        
        // 
        virtual T& get(const uint32_t& I = 0u) override { return SetBase_T<vkh::VsGeometryInstance>::get(I); };
        virtual const T& get(const uint32_t& I = 0u) const override { return SetBase_T<vkh::VsGeometryInstance>::get(I); };

        // 
        T& operator[](const uint32_t& I) { return this->get(I); };
        const T& operator[](const uint32_t& I) const { return this->get(I); };
    };

};