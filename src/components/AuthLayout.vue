<template>
  <div class="auth-container">
    <div class="auth-content">
      <div class="auth-card">
        <!-- Logo 区域 -->
        <div class="logo-section">
          <div class="logo-icon">
            <img :src="`/logo.png?v=${Date.now()}`" alt="智能低空交通系统" class="logo-image" />
          </div>
          <!-- 标题插槽，支持自定义标题 -->
          <slot name="title">
            <h1 class="auth-title">智能低空交通系统</h1>
          </slot>
        </div>

        <!-- 表单内容插槽 -->
        <div class="form-section">
          <slot name="form"></slot>
        </div>

        <!-- 底部链接插槽 -->
        <div class="bottom-link" v-if="$slots.bottomLink">
          <slot name="bottomLink"></slot>
        </div>

        <!-- 额外内容插槽（如测试账号信息） -->
        <div class="extra-content" v-if="$slots.extra">
          <slot name="extra"></slot>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
// 无需额外的脚本逻辑，完全依靠插槽来组织内容
</script>

<style scoped>
/* Apple-style minimal auth layout */
.auth-container {
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(180deg, #f5f7fa 0%, #ffffff 100%);
  font-family: -apple-system, BlinkMacSystemFont, 'SF Pro Display', 'Segoe UI', Roboto, sans-serif;
  padding: 20px;
  position: relative;
  z-index: 1;
}

.auth-content {
  width: 100%;
  max-width: 380px;
}

.auth-card {
  background: rgba(255, 255, 255, 0.9);
  backdrop-filter: blur(20px);
  border-radius: 16px;
  padding: 48px 40px 40px;
  box-shadow:
    0 1px 3px rgba(0, 0, 0, 0.02),
    0 8px 24px rgba(0, 0, 0, 0.04);
  border: 1px solid rgba(255, 255, 255, 0.8);
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.auth-card:hover {
  transform: translateY(-2px);
  box-shadow:
    0 4px 12px rgba(0, 0, 0, 0.03),
    0 16px 48px rgba(0, 0, 0, 0.06);
}

.logo-section {
  text-align: center;
  margin-bottom: 40px;
}

.logo-icon {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  margin-bottom: 20px;
  transition: all 0.3s ease;
}

.logo-icon:hover {
  transform: scale(1.05);
}

.logo-image {
  width: 120px;
  height: auto;
  object-fit: contain;
  border-radius: 12px;
}

.auth-title {
  font-size: 24px;
  font-weight: 600;
  color: #1d1d1f;
  margin: 0;
  letter-spacing: -0.5px;
  line-height: 1.2;
}

.form-section {
  margin-bottom: 24px;
}

.bottom-link {
  text-align: center;
  font-size: 14px;
  color: #86868b;
  margin-top: 24px;
}

.extra-content {
  margin-bottom: 24px;
}

/* Responsive design */
@media (max-width: 480px) {
  .auth-container {
    padding: 16px;
  }

  .auth-card {
    padding: 40px 24px 32px;
  }

  .auth-title {
    font-size: 22px;
  }
}

/* Dark mode support */
@media (prefers-color-scheme: dark) {
  .auth-container {
    background: linear-gradient(180deg, #1c1c1e 0%, #000000 100%);
  }

  .auth-card {
    background: rgba(44, 44, 46, 0.9);
    border: 1px solid rgba(84, 84, 88, 0.6);
  }

  .auth-title {
    color: #f2f2f7;
  }

  .bottom-link {
    color: #98989d;
  }
}
</style>