import requests
from bs4 import BeautifulSoup
import csv
from urllib.parse import urljoin


def parse_anime_page(url):
    """
    Парсит страницу аниме и извлекает нужные данные
    """
    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36'
    }

    try:
        response = requests.get(url, headers=headers)
        response.raise_for_status()
        soup = BeautifulSoup(response.text, 'html.parser')

        # Название аниме
        title_tag = soup.find('h1', class_='anime')
        title = title_tag.get_text(strip=True) if title_tag else 'Название не найдено'
        title = title[7:]
        
        if soup.find('span', {'title': 'language: english'}):
            alt_title_tag = soup.find('label', {'itemprop': 'alternateName'})
            title = alt_title_tag.get_text(strip=True) if alt_title_tag else title

        # Ссылка на картинку (в 1м формате)
        image_tag = soup.find('div', class_='image').find('img') if soup.find('div', class_='image') else None
        image_url = urljoin(url,
                            image_tag['src']) if image_tag and 'src' in image_tag.attrs else 'Изображение не найдено'

        # Описание
        description_tag = soup.find('div', {'itemprop': 'description'})
        if description_tag:
            # Get text with <br> replaced by newlines
            description = description_tag.get_text(strip=False)
            # Remove everything after "Source:"
            description = description.split('~ translated and adapted ')[0].strip()
            description = description.split('~ translated and adapted ')[0].strip()
            description = description.split('— written by ')[0].strip()
            description = description.split('Source:')[0].strip()
            description = description.split('Note:')[0].strip()
        else:
            description = 'Описание не найдено'

        # Количество серий
        episodes_tag = soup.find('span', {'itemprop': 'numberOfEpisodes'})
        episodes = episodes_tag.get_text(strip=True) if episodes_tag else 1

        return {
            'title': title,
            'image_url': image_url,
            'description': description,
            'episodes': episodes
        }

    except Exception as e:
        print(f"Ошибка при парсинге страницы {url}: {e}")
        return None


def save_to_csv(data, filename='anime_data.csv'):
    """
    Сохраняет данные в CSV файл
    """
    with open(filename, 'w', newline='', encoding='utf-8') as csvfile:
        fieldnames = ['title', 'image_url', 'description', 'episodes']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(data)


def main(num):
    base_url = 'https://anidb.net'
    anime_list_url = 'https://anidb.net/anime/?h=1&noalias=1&orderby.average=0.2&orderby.name=2.1&orderby.rating=1.2&view=list&page=10'

    headers = {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36'
    }

    try:
        # Получаем список аниме
        response = requests.get(anime_list_url, headers=headers)
        response.raise_for_status()
        soup = BeautifulSoup(response.text, 'html.parser')

        # Находим ссылки на аниме (первые 10 для примера)
        anime_links = []
        title_list = soup.select('td.name')
        for link in title_list[:num]:
            anime_links.append(urljoin(base_url, link.find('a')['href']))

        # Парсим каждое аниме
        anime_data = []
        for link in anime_links:
            print(f"Парсим {link}...")
            data = parse_anime_page(link)
            if data:
                anime_data.append(data)

        # Сохраняем данные
        save_to_csv(anime_data)
        print(f"Данные сохранены в anime_data.csv. Всего записей: {len(anime_data)}")

    except Exception as e:
        print(f"Ошибка: {e}")


if __name__ == '__main__':
    main(1)
