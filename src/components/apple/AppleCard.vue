<template>
  <div
    :class="[
      'apple-card',
      {
        'apple-card--hoverable': hoverable,
        'apple-card--clickable': clickable,
        'apple-card--elevated': elevated,
        'apple-card--glass': glass
      }
    ]"
    @click="handleClick"
  >
    <div v-if="$slots.header || title" class="apple-card__header">
      <slot name="header">
        <h3 v-if="title" class="apple-card__title">{{ title }}</h3>
        <p v-if="subtitle" class="apple-card__subtitle">{{ subtitle }}</p>
      </slot>
    </div>

    <div class="apple-card__body">
      <slot />
    </div>

    <div v-if="$slots.footer" class="apple-card__footer">
      <slot name="footer" />
    </div>
  </div>
</template>

<script setup>
import { defineProps, defineEmits } from 'vue'

const props = defineProps({
  title: {
    type: String,
    default: ''
  },
  subtitle: {
    type: String,
    default: ''
  },
  hoverable: {
    type: Boolean,
    default: false
  },
  clickable: {
    type: Boolean,
    default: false
  },
  elevated: {
    type: Boolean,
    default: false
  },
  glass: {
    type: Boolean,
    default: false
  }
})

const emit = defineEmits(['click'])

const handleClick = (event) => {
  if (props.clickable) {
    emit('click', event)
  }
}
</script>

<style scoped>
.apple-card {
  background-color: var(--color-bg-primary);
  border: 1px solid var(--color-border-subtle);
  border-radius: var(--radius-lg);
  box-shadow: var(--shadow-card);
  overflow: hidden;
  transition: var(--transition-card);
}

.apple-card--hoverable:hover {
  transform: translateY(-2px);
  box-shadow: var(--shadow-card-hover);
  border-color: var(--color-border-hover);
}

.apple-card--clickable {
  cursor: pointer;
}

.apple-card--clickable:active {
  transform: translateY(0) scale(0.99);
}

.apple-card--elevated {
  box-shadow: var(--shadow-elevated);
}

.apple-card--glass {
  background: var(--glass-bg);
  backdrop-filter: var(--glass-blur);
  -webkit-backdrop-filter: var(--glass-blur);
  border-color: var(--glass-border);
  box-shadow: var(--glass-shadow);
}

/* Header */
.apple-card__header {
  padding: var(--space-6);
  border-bottom: 1px solid var(--color-border-subtle);
}

.apple-card__title {
  font-size: var(--font-size-xl);
  font-weight: 600;
  color: var(--color-text-primary);
  margin: 0;
  line-height: var(--line-height-compact);
}

.apple-card__subtitle {
  font-size: var(--font-size-sm);
  color: var(--color-text-secondary);
  margin: var(--space-2) 0 0;
  line-height: var(--line-height-normal);
}

/* Body */
.apple-card__body {
  padding: var(--space-6);
}

.apple-card__header + .apple-card__body {
  padding-top: var(--space-6);
}

/* Footer */
.apple-card__footer {
  padding: var(--space-4) var(--space-6);
  border-top: 1px solid var(--color-border-subtle);
  background-color: var(--color-bg-secondary);
  display: flex;
  align-items: center;
  gap: var(--space-3);
}

/* Responsive */
@media (max-width: 768px) {
  .apple-card__header,
  .apple-card__body {
    padding: var(--space-4);
  }

  .apple-card__footer {
    padding: var(--space-3) var(--space-4);
  }
}
</style>
