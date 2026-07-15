# Quick Start Guide

## Step 1: Install from Visual Studio Marketplace

Download and install **LazyBug Copilot** from the [Visual Studio Marketplace](https://marketplace.visualstudio.com/items?itemName=IxSoftware.lazybug2026).

After installation, restart Visual Studio to activate the extension.

## Step 2: Open LazyBug Chat Panel

From the Visual Studio menu bar, select:

```
View → Other Windows → LazyBug Chat
```

The LazyBug chat panel will appear, typically docked on the right side of the IDE.

> **Tip:** You can also bind the `View.LazyBugChat` command to a keyboard shortcut for quick access:
> 1. Go to `Tools → Options → Environment → Keyboard`.
> 2. In the **Show commands containing** box, type `View.LazyBugChat`.
> 3. Select the command and assign your preferred shortcut keys (e.g., `Ctrl+Shift+L`).
> 4. Click **Assign** and then **OK**.

## Step 3: Verify API Key for Existing Providers

LazyBug comes with several pre-configured providers. To verify or update an API key for an existing provider:

1. Open the **Settings** menu in the LazyBug chat panel.
2. Select **Provider & API**.
3. Choose the provider you want to verify.
4. Paste your API key and press **Enter**.
5. If verification succeeds, the red indicator next to the Provider name will turn green.

<video src="../media/verify_existing_provider.mp4" controls="controls" width="100%"></video>

## Step 4: Add a New Provider

To add a custom LLM provider:

1. Open the **Settings** menu in the LazyBug chat panel.
2. Select **Provider & API**.
3. Click the **"+"** button below all existing providers to add a new provider.
4. Fill in: **Name**, **API Endpoint**, and select **API Format** (if unsure, choose **OpenAI** by default).
5. You need to add at least one API under this provider before you can verify the API key.
   - Click the **"+"** button on the right side of **APIs** to add an API.
   - Enter at least the **API Name** and **Model Identifier** to ensure it is usable.
6. Finally, paste your API key and press **Enter** to start verification. If verification succeeds, the red indicator next to the Provider name will turn green.

<video src="../media/add_new_provider.mp4" controls="controls" width="100%"></video>

---

Once your provider is configured, you can select an API under this Provider in the chat panel and start chatting with the AI assistant.