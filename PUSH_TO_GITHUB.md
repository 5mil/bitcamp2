# How to Push Bitcamp to GitHub

## Step 1: Create a GitHub Repository

1. Go to https://github.com/new
2. Name it `bitcamp` (or your preferred name)
3. Choose:
   - Public (to share)
   - Add LICENSE: NO (we have one)
   - Add README: NO (we have one)
4. Click "Create repository"

## Step 2: Push Your Code

```bash
cd bitcamp_github

# Initialize git (if not already done)
git init

# Add all files
git add .

# Commit
git commit -m "Initial commit: Bitcamp v1 blockchain"

# Add GitHub as remote (replace YOUR_USERNAME and bitcamp with your details)
git remote add origin https://github.com/YOUR_USERNAME/bitcamp.git

# Push to GitHub
git branch -M main
git push -u origin main
```

## Step 3: Verify

1. Go to https://github.com/YOUR_USERNAME/bitcamp
2. You should see all files and the README
3. GitHub Actions should trigger automatically on push
4. Check "Actions" tab to see builds running

## Step 4: Enable Releases

1. Go to your repo → Settings
2. Under "General", check "Discussions" (optional)
3. The GitHub Actions workflow will automatically create releases when you tag a version

To tag and release:
```bash
git tag v1.0.0
git push origin v1.0.0
```

GitHub Actions will build for all platforms and create a release automatically.

## Step 5: Badges (Optional)

Add these to your README (replace YOUR_USERNAME):

```markdown
[![Build Status](https://github.com/YOUR_USERNAME/bitcamp/actions/workflows/build.yml/badge.svg)](https://github.com/YOUR_USERNAME/bitcamp/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
```

## What Happens Next

✅ GitHub Actions builds automatically for:
- Ubuntu (Linux x64)
- macOS (Intel x64)
- Windows (x64)

✅ Artifacts stored as:
- `bitcamp-linux-x64`
- `bitcamp-macos-x64`
- `bitcamp-windows-x64.exe`

✅ When you tag a version (e.g., `v1.0.0`), all three binaries are released automatically.

## Tips

- **For continuous integration**: Every push to main triggers a build
- **For releases**: Tag with `v*` (e.g., `v1.0.1`) and GitHub creates a release with all binaries
- **To test locally before pushing**: Run `make clean && make` and verify `./bitcamp` works

## Troubleshooting GitHub Actions

If builds fail:
1. Go to Actions tab
2. Click on the failed job
3. Check logs for error messages (usually missing dependencies)
4. Update `.github/workflows/build.yml` if needed

## Next: Add More Features

Once in GitHub, consider adding:
- P2P networking layer
- REST API
- Web UI (React + WebSocket)
- Docker Hub integration
- CI/CD testing

See CONTRIBUTING.md for the roadmap!
