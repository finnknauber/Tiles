/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        bg: {
          400: "#07060a",
          500: "#0e0d13",
          600: "#201d27",
        }
      },
      boxShadow: {
        "center-lg": "0 0 15px -3px rgb(0 0 0 / 0.1), 0 0 6px -4px rgb(0 0 0 / 0.1)"
      }
    },
  },
  plugins: [],
}