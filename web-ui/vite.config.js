import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react-swc'
import { VitePWA } from 'vite-plugin-pwa'


// https://vitejs.dev/config/
export default defineConfig({
  plugins: [
    react(),
    VitePWA({ 
      registerType: 'autoUpdate',
      includeAssets: ['favicon.ico', 'apple-touch-icon-180x180.png', 'maskable-icon-512x512.png'],
      manifest: {
        name: 'OpenPAP',
        id: 'org.openpap.settings',
        short_name: 'OpenPAP',
        description: 'Open Source CPAP/BiPAP Machine',
        theme_color: '#007d7d',
        background_color: '#000000',
        "icons": [
          {
            "src": "maskable-icon.png",
            "sizes": "512x512",
            "type": "image/png",
            "purpose": "maskable"
          }
        ]
      }
    })
  ],
  base: '/settings/',
})
