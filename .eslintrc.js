module.exports = {
  env: {
    browser: true,
    node: true
  },
  extends: [
    'eslint:recommended',
    'plugin:vue/vue3-recommended'
  ],
  globals: {
    AMap: 'readonly',
    defineProps: 'readonly',
    defineEmits: 'readonly'
  },
  rules: {
    'no-unused-vars': 'warn',
    'vue/multi-word-component-names': 'off'
  },
  ignorePatterns: [
    'tests/**',
    'build/**',
    'dist/**',
    'node_modules/**'
  ]
};
