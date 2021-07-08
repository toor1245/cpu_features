import { defineUserConfig } from 'vuepress'
import type { DefaultThemeOptions } from 'vuepress'

export default defineUserConfig<DefaultThemeOptions>({
    base: '/cpu_features/',
    themeConfig: {
        logo: 'https://vuejs.org/images/logo.png',
        navbar: [
            { text: 'Home', link: '/README.md' },
            { text: 'Guide', link: '/guide/getting_started.md' },
            { text: 'Contributing', link: '/CONTRIBUTING.md' },
        ],
    },
    lang: 'en-US',
    title: 'cpu features',
    description: 'A cross platform C99 library to get cpu features at runtime',
})
