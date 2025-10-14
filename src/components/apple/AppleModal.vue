<template>
  <Teleport to="body">
    <Transition name="apple-modal-fade">
      <div
        v-if="modelValue"
        class="apple-modal-overlay"
        @click="handleOverlayClick"
      >
        <Transition name="apple-modal-scale">
          <div
            v-if="modelValue"
            :class="['apple-modal', `apple-modal--${size}`]"
            role="dialog"
            aria-modal="true"
            @click.stop
          >
            <!-- Header -->
            <div v-if="$slots.header || title || showClose" class="apple-modal__header">
              <slot name="header">
                <h2 v-if="title" class="apple-modal__title">{{ title }}</h2>
              </slot>
              <button
                v-if="showClose"
                type="button"
                class="apple-modal__close"
                aria-label="关闭"
                @click="handleClose"
              >
                ✕
              </button>
            </div>

            <!-- Body -->
            <div class="apple-modal__body">
              <slot />
            </div>

            <!-- Footer -->
            <div v-if="$slots.footer" class="apple-modal__footer">
              <slot name="footer" />
            </div>
          </div>
        </Transition>
      </div>
    </Transition>
  </Teleport>
</template>

<script setup>
import { watch, onMounted, onUnmounted } from 'vue'

const props = defineProps({
  modelValue: {
    type: Boolean,
    required: true
  },
  title: {
    type: String,
    default: ''
  },
  size: {
    type: String,
    default: 'medium',
    validator: (value) => ['small', 'medium', 'large', 'full'].includes(value)
  },
  showClose: {
    type: Boolean,
    default: true
  },
  closeOnOverlay: {
    type: Boolean,
    default: true
  },
  closeOnEscape: {
    type: Boolean,
    default: true
  }
})

const emit = defineEmits(['update:modelValue', 'close', 'open'])

const handleClose = () => {
  emit('update:modelValue', false)
  emit('close')
}

const handleOverlayClick = () => {
  if (props.closeOnOverlay) {
    handleClose()
  }
}

const handleEscape = (event) => {
  if (props.closeOnEscape && event.key === 'Escape' && props.modelValue) {
    handleClose()
  }
}

watch(() => props.modelValue, (newValue) => {
  if (newValue) {
    document.body.style.overflow = 'hidden'
    emit('open')
  } else {
    document.body.style.overflow = ''
  }
})

onMounted(() => {
  if (props.closeOnEscape) {
    document.addEventListener('keydown', handleEscape)
  }
})

onUnmounted(() => {
  document.removeEventListener('keydown', handleEscape)
  document.body.style.overflow = ''
})
</script>

<style scoped>
.apple-modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  z-index: 9999;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: var(--space-4);
  background-color: rgba(0, 0, 0, 0.5);
  backdrop-filter: blur(8px);
  -webkit-backdrop-filter: blur(8px);
  overflow-y: auto;
}

.apple-modal {
  position: relative;
  background-color: var(--color-bg-primary);
  border-radius: var(--radius-xl);
  box-shadow: var(--shadow-modal);
  display: flex;
  flex-direction: column;
  max-height: calc(100vh - var(--space-8));
  overflow: hidden;
}

/* Sizes */
.apple-modal--small {
  width: 100%;
  max-width: 400px;
}

.apple-modal--medium {
  width: 100%;
  max-width: 600px;
}

.apple-modal--large {
  width: 100%;
  max-width: 900px;
}

.apple-modal--full {
  width: calc(100vw - var(--space-8));
  height: calc(100vh - var(--space-8));
  max-width: none;
  max-height: none;
}

/* Header */
.apple-modal__header {
  position: relative;
  padding: var(--space-6);
  border-bottom: 1px solid var(--color-border-subtle);
}

.apple-modal__title {
  font-size: var(--font-size-2xl);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0;
  padding-right: var(--space-10);
  line-height: var(--line-height-compact);
}

.apple-modal__close {
  position: absolute;
  top: var(--space-6);
  right: var(--space-6);
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: var(--font-size-xl);
  color: var(--color-text-secondary);
  background-color: var(--color-bg-secondary);
  border: none;
  border-radius: 50%;
  cursor: pointer;
  transition: var(--transition-fast);
}

.apple-modal__close:hover {
  color: var(--color-text-primary);
  background-color: var(--color-bg-tertiary);
  transform: scale(1.05);
}

.apple-modal__close:active {
  transform: scale(0.95);
}

/* Body */
.apple-modal__body {
  padding: var(--space-6);
  overflow-y: auto;
  flex: 1;
}

.apple-modal__header + .apple-modal__body {
  padding-top: var(--space-6);
}

/* Footer */
.apple-modal__footer {
  padding: var(--space-4) var(--space-6);
  border-top: 1px solid var(--color-border-subtle);
  background-color: var(--color-bg-secondary);
  display: flex;
  align-items: center;
  justify-content: flex-end;
  gap: var(--space-3);
}

/* Animations */
.apple-modal-fade-enter-active,
.apple-modal-fade-leave-active {
  transition: opacity var(--duration-normal) var(--easing-default);
}

.apple-modal-fade-enter-from,
.apple-modal-fade-leave-to {
  opacity: 0;
}

.apple-modal-scale-enter-active,
.apple-modal-scale-leave-active {
  transition: all var(--duration-normal) var(--easing-spring);
}

.apple-modal-scale-enter-from,
.apple-modal-scale-leave-to {
  opacity: 0;
  transform: scale(0.95);
}

/* Responsive */
@media (max-width: 768px) {
  .apple-modal-overlay {
    padding: var(--space-2);
  }

  .apple-modal {
    max-height: calc(100vh - var(--space-4));
  }

  .apple-modal--small,
  .apple-modal--medium,
  .apple-modal--large {
    width: 100%;
    max-width: none;
  }

  .apple-modal__header,
  .apple-modal__body {
    padding: var(--space-4);
  }

  .apple-modal__footer {
    padding: var(--space-3) var(--space-4);
    flex-direction: column-reverse;
  }

  .apple-modal__footer > * {
    width: 100%;
  }
}
</style>
