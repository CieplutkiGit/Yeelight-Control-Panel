const prefersReducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;

const revealItems = document.querySelectorAll('.reveal');
if (prefersReducedMotion || !('IntersectionObserver' in window)) {
  revealItems.forEach((item) => item.classList.add('is-visible'));
} else {
  const revealObserver = new IntersectionObserver((entries, observer) => {
    entries.forEach((entry) => {
      if (!entry.isIntersecting) return;
      entry.target.classList.add('is-visible');
      observer.unobserve(entry.target);
    });
  }, { threshold: 0.14, rootMargin: '0px 0px -30px' });

  revealItems.forEach((item) => revealObserver.observe(item));
}

const room = document.querySelector('.room-art');
const hangingLamp = document.querySelector('.hanging-lamp-control');
const tableLamp = document.querySelector('.table-lamp-control');
const lampStatus = document.querySelector('#lamp-status');
const lampStates = ['off', 'warm', 'neutral', 'cool'];
const lampLabels = { off: 'off', warm: 'warm', neutral: 'neutral', cool: 'cool' };

hangingLamp?.addEventListener('click', () => {
  if (!room) return;
  const currentState = room.dataset.lampState || 'warm';
  const nextState = lampStates[(lampStates.indexOf(currentState) + 1) % lampStates.length];
  room.dataset.lampState = nextState;
  if (lampStatus) lampStatus.textContent = `Hanging lamp · ${lampLabels[nextState]}`;
});

tableLamp?.addEventListener('click', () => {
  if (!room || !tableLamp) return;
  const isOff = room.dataset.tableLamp === 'off';
  room.dataset.tableLamp = isOff ? 'on' : 'off';
  tableLamp.setAttribute('aria-pressed', String(isOff));
  tableLamp.setAttribute('aria-label', isOff ? 'Turn table lamp off' : 'Turn table lamp on');
});

const appWindow = document.querySelector('.app-window');
const brightness = document.querySelector('#brightness');
const brightnessValue = document.querySelector('#brightnessValue');
const temperature = document.querySelector('#temperature');
const tempValue = document.querySelector('#tempValue');
const powerButton = document.querySelector('#powerButton');
const connectionStatus = document.querySelector('#connectionStatus');

const updatePreviewLight = () => {
  if (!appWindow || !brightness || !temperature) return;
  const brightnessAmount = Number(brightness.value);
  const temperatureAmount = (Number(temperature.value) - 1700) / (6500 - 1700);
  const warm = [244, 185, 80];
  const cool = [191, 216, 241];
  const rgb = warm.map((value, index) => Math.round(value + (cool[index] - value) * temperatureAmount));
  appWindow.style.setProperty('--preview-bulb', `rgb(${rgb.join(',')})`);
  appWindow.style.setProperty('--preview-glow', `${Math.round(8 + brightnessAmount / 2)}px`);
  if (brightnessValue) brightnessValue.textContent = `${brightnessAmount}%`;
  if (tempValue) tempValue.textContent = `${temperature.value} K`;
};

brightness?.addEventListener('input', updatePreviewLight);
temperature?.addEventListener('input', updatePreviewLight);

powerButton?.addEventListener('click', () => {
  if (!appWindow || !connectionStatus) return;
  const isOff = appWindow.classList.toggle('is-off');
  powerButton.classList.toggle('is-on', !isOff);
  powerButton.setAttribute('aria-pressed', String(!isOff));
  connectionStatus.textContent = isOff ? '○ Standby' : '● Connected';
  connectionStatus.style.color = isOff ? '#a5aaa5' : '#72b963';
});

document.querySelectorAll('.device').forEach((device) => {
  device.addEventListener('click', () => {
    document.querySelectorAll('.device').forEach((item) => {
      item.classList.remove('active');
      item.setAttribute('aria-pressed', 'false');
    });
    device.classList.add('active');
    device.setAttribute('aria-pressed', 'true');
    const selectedDevice = document.querySelector('#selectedDevice');
    if (selectedDevice) selectedDevice.textContent = device.dataset.device || 'Living Room';
  });
});

document.querySelector('[data-copy]')?.addEventListener('click', async (event) => {
  const button = event.currentTarget;
  const text = document.querySelector('.code-card code')?.textContent ?? '';
  try {
    await navigator.clipboard.writeText(text);
    button.textContent = 'Copied';
    setTimeout(() => { button.textContent = 'Copy'; }, 1400);
  } catch {
    button.textContent = 'Select text';
  }
});

updatePreviewLight();
